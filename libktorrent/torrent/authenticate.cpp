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
#include <util/log.h>
#include <mse/streamsocket.h>
#include "authenticate.h"
#include "ipblocklist.h"
#include "peermanager.h"

namespace bt
{

	Authenticate::Authenticate(const QString & ip,Uint16 port,
							   const SHA1Hash & info_hash,const PeerID & peer_id,PeerManager* pman) 
	: info_hash(info_hash),our_peer_id(peer_id),pman(pman)
	{
		finished = succes = false;
		sock = new mse::StreamSocket();
		host = ip;
		this->port = port;

		Out(SYS_CON|LOG_NOTICE) << "Initiating connection to " << host << endl;

		if (sock->connectTo(host,port))
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

	Authenticate::~Authenticate()
	{
	}
	
	void Authenticate::onReadyWrite()
	{
//		Out() << "Authenticate::onReadyWrite()" << endl;
		if (sock->connectSuccesFull())
		{
			connected();
		}
		else
		{
			onFinish(false);
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
		if (pman)
			pman->peerAuthenticated(this,succes);
	}
	
	void Authenticate::handshakeRecieved(bool full)
	{
		const Uint8* hs = handshake;
	//	Out() << "Authenticate::handshakeRecieved" << endl;
		IPBlocklist& ipfilter = IPBlocklist::instance();
			//Out() << "Dodo " << pp.ip << endl;
		if (ipfilter.isBlocked(host))
		{
			onFinish(false);
			return;
		}
		
		SHA1Hash rh(hs+28);
		if (rh != info_hash)
		{
			Out() << "Wrong info_hash : " << rh.toString() << endl;
			onFinish(false);
			return;
		}
		
		char tmp[21];
		tmp[20] = '\0';
		memcpy(tmp,hs+48,20);
		peer_id = PeerID(tmp);
		
		if (our_peer_id == peer_id /*|| peer_id.startsWith("Yoda")*/)
		{
			Out(SYS_CON|LOG_DEBUG) << "Lets not connect to our selves " << endl;
			onFinish(false);
			return;
		}
		
		// check if we aren't already connected to the client
		if (pman->connectedTo(peer_id))
		{
			Out(SYS_CON|LOG_NOTICE) << "Already connected to " << peer_id.toString() << endl;
			onFinish(false);
			return;
		}
		
		// only finish when the handshake was fully received
		if (full)
			onFinish(true);
	}


	mse::StreamSocket* Authenticate::takeSocket()
	{
		mse::StreamSocket* s = sock;
		sock = 0;
		return s;
	}
	
	void Authenticate::onPeerManagerDestroyed()
	{
	//	Out(SYS_CON|LOG_NOTICE) << "Authenticate::onPeerManagerDestroyed()" << endl;
		pman = 0;
		if (finished)
			return;
		
		onFinish(false);
	}
	
}
#include "authenticate.moc"
