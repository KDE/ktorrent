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

#include <qserversocket.h>
#include <mse/streamsocket.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <net/portlist.h>
#include <mse/encryptedserverauthenticate.h>
#include "globals.h"
#include "torrent.h"
#include "server.h"
#include "peermanager.h"
#include "serverauthenticate.h"
#include "ipblocklist.h"
#include "authenticationmonitor.h"


namespace bt
{

	class ServerSocket : public QServerSocket
	{
		Server* srv;
	public:	
		ServerSocket(Server* srv,Uint16 port) : QServerSocket(port),srv(srv)
		{
			QSocketDevice* sd = socketDevice();
			if (sd)
				sd->setAddressReusable(true);
		}
		
		virtual ~ServerSocket()
		{}
		
		virtual void newConnection(int socket) 
		{
			srv->newConnection(socket);
		}
	};


	

	Server::Server(Uint16 port) : sock(0),port(0)
	{
		changePort(port);
		encryption = false;
		allow_unencrypted = true;
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
		if (p == port)
			return;

		
		if (sock && sock->ok())
			Globals::instance().getPortList().removePort(port,net::TCP);
		
		port = p;
		delete sock;
		sock = new ServerSocket(this,port);
		if (isOK())
			Globals::instance().getPortList().addNewPort(port,net::TCP,true);
	}

	void Server::addPeerManager(PeerManager* pman)
	{
		peer_managers.append(pman);
	}
	
	void Server::removePeerManager(PeerManager* pman)
	{
		peer_managers.remove(pman);
	}
	
	void Server::newConnection(int socket)
	{
		mse::StreamSocket* s = new mse::StreamSocket(socket);
		if (peer_managers.count() == 0)
		{
			s->close();
			delete s;
		}
		else
		{
			IPBlocklist& ipfilter = IPBlocklist::instance();
			QString IP(s->getRemoteIPAddress());
			if (ipfilter.isBlocked( IP ))
			{
				delete s;
				return;
			}
			
			ServerAuthenticate* auth = 0;
			
			if (encryption)
				auth = new mse::EncryptedServerAuthenticate(s,this);
			else
				auth = new ServerAuthenticate(s,this);
			
			AuthenticationMonitor::instance().add(auth);
		}
	}
	
	void Server::close()
	{
		delete sock;
		sock= 0;
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
			if (pm && pm->getTorrent().getInfoHash() == hash)
			{
				if (!pm->isStarted())
					return 0;
				else
					return pm;
			}
			i++;
		}
		return 0;
	}
	
	bool Server::findInfoHash(const SHA1Hash & skey,SHA1Hash & info_hash)
	{
		Uint8 buf[24];
		memcpy(buf,"req2",4);
		QPtrList<PeerManager>::iterator i = peer_managers.begin();
		while (i != peer_managers.end())
		{
			PeerManager* pm = *i;
			memcpy(buf+4,pm->getTorrent().getInfoHash().getData(),20);
			if (SHA1Hash::generate(buf,24) == skey)
			{
				info_hash = pm->getTorrent().getInfoHash();
				return true;
			}
			i++;
		}
		return false;
	}

	void Server::onError(int)
	{
	}
	
	
	void Server::enableEncryption(bool allow_unencrypted)
	{
		encryption = true;
		this->allow_unencrypted = allow_unencrypted;
	}	
	
	void Server::disableEncryption()
	{
		encryption = false;
	}
}

#include "server.moc"
