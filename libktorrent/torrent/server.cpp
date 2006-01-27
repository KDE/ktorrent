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
using namespace KNetwork;
#else
#include <qserversocket.h>
#endif


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
#ifndef USE_KNETWORK_SOCKET_CLASSES
	class ServerSocket : public QServerSocket
	{
		Server* srv;
	public:	
		ServerSocket(Server* srv,Uint16 port) : QServerSocket(port),srv(srv)
		{
			socketDevice()->setAddressReusable(true);
		}
		
		virtual ~ServerSocket()
		{}
		
		virtual void newConnection(int socket) 
		{
			srv->newConnection(socket);
		}
	};

#endif
	

	Server::Server(Uint16 port) : sock(0),port(0)
	{
		pending.setAutoDelete(false);
		changePort(port);
	}


	Server::~Server()
	{
		pending.setAutoDelete(true);
#ifdef USE_KNETWORK_SOCKET_CLASSES
		sock->close();
#endif
		delete sock;
	}

	bool Server::isOK() const
	{
#ifdef USE_KNETWORK_SOCKET_CLASSES
		return sock->error() == KSocketBase::NoError;
#else
		return sock->ok();
#endif
	}

	void Server::changePort(Uint16 p)
	{
		if (p == port)
			return;

		port = p;
		delete sock;
#ifdef USE_KNETWORK_SOCKET_CLASSES
		sock = new KServerSocket();
		sock->setIPv6Only(true);
		connect(sock, SIGNAL(readyAccept()), this, SLOT(newConnection()));
		connect(sock, SIGNAL(gotError(int)), this, SLOT(onError(int )));
		sock->setFamily(KResolver::IPv4Family);
		sock->setAddress(QString::number(port));
		sock->setAcceptBuffered(true);
		sock->setAddressReuseable(true);
		sock->listen();
#else
		sock = new ServerSocket(this,port);
#endif
	}

	void Server::addPeerManager(PeerManager* pman)
	{
		peer_managers.append(pman);
	}
	
	void Server::removePeerManager(PeerManager* pman)
	{
		peer_managers.remove(pman);
	}

	void Server::newConnection()
	{
#ifdef USE_KNETWORK_SOCKET_CLASSES
		KActiveSocketBase* b = sock->accept();
		KBufferedSocket* conn = dynamic_cast<KBufferedSocket*>(b);

		if (!conn)
		{
			Out() << "Conn = 0 !" << endl;
			if (dynamic_cast<KStreamSocket*>(b))
				Out() << "Stream socket !!" << endl;
			return;
		}
		
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
#endif
	}

	void Server::newConnection(int socket)
	{
#ifndef USE_KNETWORK_SOCKET_CLASSES
		QSocket* s = new QSocket();
		s->setSocket(socket);
		if (peer_managers.count() == 0)
		{
			s->close();
			delete s;
		}
		else
		{
			IPBlocklist& ipfilter = IPBlocklist::instance();
			QString IP(s->peerAddress().toString());
			if (ipfilter.isBlocked( IP ))
			{
				Out() << "Peer " << IP << " is blacklisted. Aborting connection." << endl;
				delete s;
				return;
			}
			
			ServerAuthenticate* auth = new ServerAuthenticate(s,this);
			pending.append(auth);
		}
#endif
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

	void Server::onError(int)
	{
#ifdef USE_KNETWORK_SOCKET_CLASSES
		Out() << "Server error : " << sock->errorString() << endl;
#endif
	}
	
	void Server::update()
	{
		QPtrList<ServerAuthenticate>::iterator i = pending.begin();
		while (i != pending.end())
		{
			ServerAuthenticate* auth = *i;
			if (auth->isFinished())
			{
				delete auth;
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
