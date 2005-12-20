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

#ifdef USE_KNETWORK_SOCKET_CLASSES
	ServerAuthenticate::ServerAuthenticate(KNetwork::KBufferedSocket* sock,Server* server)
	: AuthenticateBase(sock),server(server)
	{
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(gotError(int)),this,SLOT(onError(int )));
	}
#else
	ServerAuthenticate::ServerAuthenticate(QSocket* sock,Server* server)
	: AuthenticateBase(sock),server(server)
	{
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
	}
#endif


	ServerAuthenticate::~ServerAuthenticate()
	{}
	
	void ServerAuthenticate::onFinish(bool succes)
	{
		if (!sock) return;
		
#ifdef USE_KNETWORK_SOCKET_CLASSES
		Out() << "Authentication(S) to " << sock->peerAddress().nodeName()
			<< " : " << (succes ? "ok" : "failure") << endl;
		disconnect(sock,SIGNAL(gotError(int)),this,SLOT(onError(int )));
#else
		Out() << "Authentication(S) to " << sock->peerAddress().toString()
			<< " : " << (succes ? "ok" : "failure") << endl;
		disconnect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
#endif
		disconnect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		
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
#ifdef USE_KNETWORK_SOCKET_CLASSES
		QString IP(sock->peerAddress().toString().section(':',3).section(']',0,0));
#else
		QString IP(sock->peerAddress().toString());
#endif
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
			
			// check if we aren't allready connected to the client
			if (pman->connectedTo(peer_id))
			{
				Out() << "Allready connected to " << peer_id.toString() << endl;
				onFinish(false);
				return;
			}
			
					
			// send handshake and finish off
			sendHandshake(rh,pman->getTorrent().getPeerID());
			onFinish(true);
			// hand over connection
			pman->newConnection(sock,peer_id);
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
