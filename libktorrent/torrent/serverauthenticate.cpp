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
#include <qsocket.h>
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

	ServerAuthenticate::ServerAuthenticate(QSocket* sock,Server* server)
	: AuthenticateBase(sock),server(server)
	{
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
	}


	ServerAuthenticate::~ServerAuthenticate()
	{}
	
	void ServerAuthenticate::onFinish(bool succes)
	{
		if (!sock) return;
		
		Out() << "Authentication(S) to " << sock->peerAddress().toString()
			<< " : " << (succes ? "ok" : "failure") << endl;
		disconnect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		disconnect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
		finished = true;
		if (!succes)
		{
			delete sock;
			sock = 0;
		}
		timer.stop();
	}

	void ServerAuthenticate::handshakeRecieved(const Uint8* hs)
	{
		IPBlocklist& ipfilter = IPBlocklist::instance();
		QString IP(sock->peerAddress().toString());
		if (ipfilter.isBlocked( IP ))
		{
			Out() << "Peer " << IP << " is blacklisted. Aborting connection." << endl;
			onFinish(false);
			return;
		}
		
		// try to find a PeerManager which has te right info hash
		SHA1Hash rh(hs+28);
		PeerManager* pman = server->findPeerManager(rh);
		if (!pman)
		{
			onFinish(false);
			return;
		}

		// check if we aren't connecting to ourself
		char tmp[21];
		tmp[20] = '\0';
		memcpy(tmp,hs+48,20);
		PeerID peer_id = PeerID(tmp);
		if (pman->getTorrent().getPeerID() == peer_id)
		{
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
}

#include "serverauthenticate.moc"
