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
#include "globals.h"
#include "torrent.h"
#include "server.h"
#include "peermanager.h"
#include "serverauthenticate.h"
#include "ipblocklist.h"



namespace bt
{
	class ServerSocket : public QServerSocket
	{
		Server* srv;
	public:
		ServerSocket(Server* srv,Uint16 port)
			: QServerSocket(port),srv(srv)
		{
		}
		 
		virtual ~ServerSocket() {}
		
	
		void newConnection(int socket)
		{
			srv->newConnection(socket);
		}
	};

	Server::Server(Uint16 port) : port(port)
	{
		pending.setAutoDelete(true);
		sock = new ServerSocket(this,port);
	}


	Server::~Server()
	{
		delete sock;
	}

	bool Server::isOK() const
	{
		return sock->ok();
	}

	void Server::changePort(Uint16 p)
	{
		if (p == sock->port())
			return;

		delete sock;
		sock = new ServerSocket(this,p);
		this->port = p;
	}

	void Server::addPeerManager(PeerManager* pman)
	{
		peer_managers.append(pman);
	}
	
	void Server::removePeerManager(PeerManager* pman)
	{
		peer_managers.remove(pman);
	}

	void Server::newConnection(int s)
	{
		QSocket* conn = new QSocket();
		conn->setSocket(s);
		if (!conn)
			return;
		
		if (peer_managers.count() == 0)
		{
			conn->close();
			delete conn;
		}
		else
		{
			IPBlocklist& ipfilter = IPBlocklist::instance();
			QString IP(conn->peerAddress().toString());
			if (ipfilter.isBlocked( IP ))
			{
				Out() << "Peer " << IP << " is blacklisted. Aborting connection." << endl;
				delete conn;
				return;
			}
			
			ServerAuthenticate* auth = new ServerAuthenticate(conn,this);
			pending.append(auth);
		}
	}

	Uint16 Server::getPortInUse() const
	{
		return port;
	}

	PeerManager* Server::findPeerManager(const SHA1Hash & hash)
	{
		QPtrList<PeerManager>::iterator i = peer_managers.begin();
		while (i != peer_managers.end())
		{
			PeerManager* pm = *i;
			if (pm->getTorrent().getInfoHash() == hash)
				return pm;
			i++;
		}
		return 0;
	}
	
	void Server::update()
	{
		QPtrList<ServerAuthenticate>::iterator i = pending.begin();
		while (i != pending.end())
		{
			ServerAuthenticate* auth = *i;
			if (auth->isFinished())
			{
				i = pending.erase(i);
			}
			else
			{
				i++;
			}
		}
	}
}

#include "server.moc"
