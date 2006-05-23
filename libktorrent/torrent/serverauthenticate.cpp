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

#include <mse/streamsocket.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <util/log.h>
#include "globals.h"
#include "server.h"
#include "peermanager.h"
#include "serverauthenticate.h"
#include "peerid.h"
#include "torrent.h"
#include "ipblocklist.h"


namespace bt
{


	ServerAuthenticate::ServerAuthenticate(mse::StreamSocket* sock,Server* server)
	: AuthenticateBase(sock),server(server)
	{
		sock->attachAuthenticate(this);
	}


	ServerAuthenticate::~ServerAuthenticate()
	{}
	
	void ServerAuthenticate::onFinish(bool succes)
	{
		if (!sock) return;
		
		Out() << "Authentication(S) to " << sock->getIPAddress() 
				<< " : " << (succes ? "ok" : "failure") << endl;
		sock->detachAuthenticate(this);
		finished = true;
		if (!succes)
		{
			sock->deleteLater();
			sock = 0;
		}
		timer.stop();
	}

	void ServerAuthenticate::handshakeRecieved(bool full)
	{
		Uint8* hs = handshake;
		IPBlocklist& ipfilter = IPBlocklist::instance();

		QString IP = sock->getIPAddress();

		if (ipfilter.isBlocked( IP ))
		{
			onFinish(false);
			return;
		}
		
		// try to find a PeerManager which has te right info hash
		SHA1Hash rh(hs+28);
		PeerManager* pman = server->findPeerManager(rh);
		if (!pman)
		{
			Out() << "Cannot find PeerManager for hash : " << rh.toString() << endl;
			onFinish(false);
			return;
		}

		if (full)
		{
			// check if we aren't connecting to ourself
			char tmp[21];
			tmp[20] = '\0';
			memcpy(tmp,hs+48,20);
			PeerID peer_id = PeerID(tmp);
			if (pman->getTorrent().getPeerID() == peer_id)
			{
				Out() << "Lets not connect to our self" << endl;
				onFinish(false);
				return;
			}
			
			// check if we aren't already connected to the client
			if (pman->connectedTo(peer_id))
			{
				Out() << "Already connected to " << peer_id.toString() << endl;
				onFinish(false);
				return;
			}
			
					
			// send handshake and finish off
			sendHandshake(rh,pman->getTorrent().getPeerID());
			onFinish(true);
			// hand over connection
			Uint32 flags = 0;
			if (supportsDHT())
				flags |= bt::DHT_SUPPORT;
			if (supportsFastExtensions())
				flags |= bt::FAST_EXT_SUPPORT;
			
			pman->newConnection(sock,peer_id,flags);
			sock = 0;
		}
		else
		{
			// if the handshake wasn't fully recieved just send our handshake
			sendHandshake(rh,pman->getTorrent().getPeerID());
		}
	}
}

#include "serverauthenticate.moc"
