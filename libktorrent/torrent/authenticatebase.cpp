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
#include <mse/streamsocket.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <kademlia/dhtbase.h>
#include "globals.h"
#include "peerid.h"
#include "authenticatebase.h"

namespace bt
{
	
	

	AuthenticateBase::AuthenticateBase(mse::StreamSocket*  s) : sock(s),finished(false),local(false)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(20000,true);
		memset(handshake,0x00,68);
		bytes_of_handshake_recieved = 0;
		ext_support = 0;
		poll_index = -1;
	}


	AuthenticateBase::~AuthenticateBase()
	{
		if (sock)
			sock->deleteLater();
	}

	void AuthenticateBase::sendHandshake(const SHA1Hash & info_hash,const PeerID & our_peer_id)
	{
	//	Out() << "AuthenticateBase::sendHandshake" << endl;
		if (!sock) return;
		
		Uint8 hs[68];
		makeHandshake(hs,info_hash,our_peer_id);
		sock->sendData(hs,68);
	}
	
	void AuthenticateBase::makeHandshake(Uint8* hs,const SHA1Hash & info_hash,const PeerID & our_peer_id)
	{
		const char* pstr = "BitTorrent protocol";
		hs[0] = 19;
		memcpy(hs+1,pstr,19);
		memset(hs+20,0x00,8);
		if (Globals::instance().getDHT().isRunning())
			hs[27] |= 0x01; // DHT support
			 
		hs[25] |= 0x10; // extension protocol
		hs[27] |= 0x04; // fast extensions
		memcpy(hs+28,info_hash.getData(),20);
		memcpy(hs+48,our_peer_id.data(),20);
	}

	void AuthenticateBase::onReadyRead()
	{
		Uint32 ba = sock->bytesAvailable();
	//	Out() << "AuthenticateBase::onReadyRead " << ba << endl;
		if (ba == 0)
		{
			onFinish(false);
			return;
		}
		
		if (!sock || finished || ba < 48)
			return;
		
		// first see if we already have some bytes from the handshake
		if (bytes_of_handshake_recieved == 0)
		{
			if (ba < 68)
			{
				// read partial
				sock->readData(handshake,ba);
				bytes_of_handshake_recieved += ba;
				if (ba >= 27 && handshake[27] & 0x01)
					ext_support |= bt::DHT_SUPPORT;
				// tell subclasses of a partial handshake
				handshakeRecieved(false);
				return;
			}
			else
			{
				// read full handshake
				sock->readData(handshake,68);
			}
		}
		else
		{
			// read remaining part
			Uint32 to_read = 68 - bytes_of_handshake_recieved;
			sock->readData(handshake + bytes_of_handshake_recieved,to_read);
		}
	
		if (handshake[0] != 19)
		{
			onFinish(false);
			return;
		}
		
		const char* pstr = "BitTorrent protocol";
		if (memcmp(pstr,handshake+1,19) != 0)
		{
			onFinish(false);
			return;
		}
		
		if (Globals::instance().getDHT().isRunning() && (handshake[27] & 0x01))
			ext_support |= bt::DHT_SUPPORT;
		
		if (handshake[27] & 0x04)
			ext_support |= bt::FAST_EXT_SUPPORT;
		
		if (handshake[25] & 0x10)
			ext_support |= bt::EXT_PROT_SUPPORT;
		
		handshakeRecieved(true);
	}

	void AuthenticateBase::onError(int)
	{
		if (finished)
			return;
		onFinish(false);
	}

	void AuthenticateBase::onTimeout()
	{
		if (finished)
			return;
		
		Out(SYS_CON|LOG_DEBUG) << "Timeout occurred" << endl;
		onFinish(false);
	}
	
	void AuthenticateBase::onReadyWrite()
	{}
}
#include "authenticatebase.moc"
