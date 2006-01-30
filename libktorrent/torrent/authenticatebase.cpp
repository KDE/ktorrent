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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifdef USE_KNETWORK_SOCKET_CLASSES
#include <kbufferedsocket.h>
#endif

#include <util/sha1hash.h>
#include <util/log.h>
#include "globals.h"
#include "peerid.h"
#include "authenticatebase.h"

namespace bt
{
#ifdef USE_KNETWORK_SOCKET_CLASSES
	AuthenticateBase::AuthenticateBase(KNetwork::KBufferedSocket* s)
#else
	AuthenticateBase::AuthenticateBase(QSocket* s)
#endif
		: sock(s),finished(false)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(20000,true);
		memset(handshake,0x00,68);
		bytes_of_handshake_recieved = 0;
	}


	AuthenticateBase::~AuthenticateBase()
	{
		delete sock;
	}

	void AuthenticateBase::sendHandshake(const SHA1Hash & info_hash,const PeerID & our_peer_id)
	{
	//	Out() << "AuthenticateBase::sendHandshake" << endl;
		if (!sock) return;
		
		Uint8 hs[68];
		const char* pstr = "BitTorrent protocol";
		hs[0] = 19;
		memcpy(hs+1,pstr,19);
		memset(hs+20,0x00,8);
		memcpy(hs+28,info_hash.getData(),20);
		memcpy(hs+48,our_peer_id.data(),20);
		
		sock->writeBlock((const char*)hs,68);
	}

	void AuthenticateBase::onReadyRead()
	{
		//Out() << "AuthenticateBase::onReadyRead" << endl;
		if (!sock || finished || sock->bytesAvailable() < 48)
			return;
		
		Uint32 ba = sock->bytesAvailable();
		
		// first see if we already have some bytes from the handshake
		if (bytes_of_handshake_recieved == 0)
		{
			if (ba < 68)
			{
				// read partial
				sock->readBlock((char*)handshake,ba);
				bytes_of_handshake_recieved += ba;
				// tell subclasses of a partial handshake
				handshakeRecieved(false);
				return;
			}
			else
			{
				// read full handshake
				sock->readBlock((char*)handshake,68);
			}
		}
		else
		{
			// read remaining part
			Uint32 to_read = 68 - bytes_of_handshake_recieved;
			sock->readBlock((char*)handshake + bytes_of_handshake_recieved,to_read);
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
		handshakeRecieved(true);
	}

	void AuthenticateBase::onError(int)
	{
		if (finished)
			return;
#ifdef USE_KNETWORK_SOCKET_CLASSES
		Out() << "Socket error : " << sock->errorString() << endl;
#endif
		onFinish(false);
	}

	void AuthenticateBase::onTimeout()
	{
		if (finished)
			return;
		
		Out() << "Timeout occured" << endl;
		onFinish(false);
	}
}
#include "authenticatebase.moc"
