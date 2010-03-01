/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "authenticate.h"
#include <util/log.h>
#include <mse/streamsocket.h>
#include <peer/accessmanager.h>
#include <net/socks.h>
#include <utp/utpsocket.h>
#include "peerconnector.h"

namespace bt
{

	Authenticate::Authenticate(const QString & ip,Uint16 port,TransportProtocol proto,
							   const SHA1Hash & info_hash,const PeerID & peer_id,PeerConnector* pcon) 
	: info_hash(info_hash),our_peer_id(peer_id),pcon(pcon),socks(0)
	{
		finished = succes = false;
		net::Address addr(ip,port);
		if (proto == TCP)
			sock = new mse::StreamSocket(addr.ipVersion());
		else
			sock = new mse::StreamSocket(new utp::UTPSocket());
		
		host = ip;
		this->port = port;

		Out(SYS_CON|LOG_NOTICE) << "Initiating connection to " << host << " via (" << (proto == TCP ? "TCP" : "UTP") << ")" << endl;
		if (net::Socks::enabled())
		{
			socks = new net::Socks(sock,addr);
			switch (socks->setup())
			{
				case net::Socks::FAILED:
					Out(SYS_CON|LOG_NOTICE) << "Failed to connect to " << host << " via socks server " << endl;
					onFinish(false);
					break;
				case net::Socks::CONNECTED:
					delete socks;
					socks = 0; 
					connected();
					break;
				default:
					break;
			}
		}
		else
		{
			if (sock->connectTo(addr))
			{
				connected();
			}
			else if (sock->connecting())
			{
				// do nothing the monitor will notify us when we are connected
			}
			else
			{
				onFinish(false);
			}
		}
	}

	Authenticate::~Authenticate()
	{
		delete socks;
	}
	
	void Authenticate::onReadyWrite()
	{
		if (!sock)
			return;
		
		if (socks)
		{
			switch (socks->onReadyToWrite())
			{
				case net::Socks::FAILED:
					Out(SYS_CON|LOG_NOTICE) << "Failed to connect to socks server " << endl;
					onFinish(false);
					break;
				case net::Socks::CONNECTED:
					delete socks;
					socks = 0; 
					connected();
					break;
				default:
					break;
			}
		}
		else if (sock->connectSuccesFull())
		{
			connected();
		}
		else
		{
			onFinish(false);
		}
	}
	
	void Authenticate::onReadyRead()
	{
		if (!sock)
			return;
		
		if (!socks)
		{
			AuthenticateBase::onReadyRead();
		}
		else
		{
			switch (socks->onReadyToRead())
			{
				case net::Socks::FAILED:
					Out(SYS_CON|LOG_NOTICE) << "Failed to connect to host via socks server " << endl;
					onFinish(false);
					break;
				case net::Socks::CONNECTED:
					// connection established, so get rid of socks shit
					delete socks;
					socks = 0; 
					connected();
					if (sock->bytesAvailable() > 0)
						AuthenticateBase::onReadyRead();
					break;
				default:
					break;
			}
		}
	}

	void Authenticate::connected()
	{
		sendHandshake(info_hash,our_peer_id);
	}

	void Authenticate::onFinish(bool succes)
	{
		Out(SYS_CON|LOG_NOTICE) << "Authentication to " << host << " : " << (succes ? "ok" : "failure") << endl;
		finished = true;
		this->succes = succes;
		
		if (!succes)
		{
			sock->deleteLater();
			sock = 0;
		}
		timer.stop();
		if (pcon)
			pcon->authenticationFinished(this,succes);
	}
	
	void Authenticate::handshakeReceived(bool full)
	{
		const Uint8* hs = handshake;
		if (!AccessManager::instance().allowed(host))
		{
			Out(SYS_CON|LOG_DEBUG) << "The IP address " << host << " is blocked " << endl;
			onFinish(false);
			return;
		}
		
		SHA1Hash rh(hs+28);
		if (rh != info_hash)
		{
			Out(SYS_CON|LOG_DEBUG) << "Wrong info_hash : " << rh.toString() << endl;
			onFinish(false);
			return;
		}
		
		if (full)
		{
			char tmp[21];
			tmp[20] = '\0';
			memcpy(tmp,hs+48,20);
			peer_id = PeerID(tmp);
			
			if (our_peer_id == peer_id)
			{
				Out(SYS_CON|LOG_DEBUG) << "Lets not connect to our selves " << endl;
				onFinish(false);
				return;
			}
			
			// only finish when the handshake was fully received
			onFinish(true);
		}
	}


	mse::StreamSocket* Authenticate::takeSocket()
	{
		mse::StreamSocket* s = sock;
		sock = 0;
		return s;
	}
	
	void Authenticate::stop()
	{
		if (finished)
			return;
		
		onFinish(false);
	}
	
	
	
}
#include "authenticate.moc"
