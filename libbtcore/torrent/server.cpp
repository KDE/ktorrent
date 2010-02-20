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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "server.h"
#include <QStringList>
#include <QHostAddress>
#include <qsocketnotifier.h>
#include <net/socket.h>
#include <mse/streamsocket.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <util/functions.h>
#include <net/portlist.h>
#include <mse/encryptedserverauthenticate.h>
#include <peer/peermanager.h>
#include <peer/serverauthenticate.h>
#include <peer/authenticationmonitor.h>
#include <peer/accessmanager.h>

#include "globals.h"
#include "torrent.h"




namespace bt
{

	

	Server::Server() : sock(0),sn(0)
	{
	}


	Server::~Server()
	{
		delete sn;
		delete sock;
	}
	
	bool Server::changePort(Uint16 p)
	{
		if (sock && p == port)
			return true;

		if (sock && sock->ok())
			Globals::instance().getPortList().removePort(port,net::TCP);
		
		delete sock;
		sock = 0;
		delete sn; 
		sn = 0;
		
		QStringList possible = bindAddresses();
		foreach (const QString & addr,possible)
		{
			if (addr.contains(":")) // IPv6
				sock = new net::Socket(true,6);
			else
				sock = new net::Socket(true,4);
			
			if (sock->bind(addr,p,true))
			{
				Out(SYS_GEN|LOG_NOTICE) << "Bound to " << addr << ":" << p << endl;
				break;
			}
			
			delete sock;
			sock = 0;
		}
		
		if (sock)
		{
			sock->setBlocking(false);
			sn = new QSocketNotifier(sock->fd(),QSocketNotifier::Read,this);
			connect(sn,SIGNAL(activated(int)),this,SLOT(readyToAccept(int)));
			Globals::instance().getPortList().addNewPort(p,net::TCP,true);
			return true;
		}
		
		return false;
	}

	void Server::readyToAccept(int )
	{
		if (!sock)
			return;

		net::Address addr;
		int fd = sock->accept(addr);
		if (fd > 0)
			newConnection(new mse::StreamSocket(fd,sock->isIPv4() ? 4 : 6));
	}
	
	void Server::close()
	{
		delete sock;
		sock = 0;
		delete sn;
		sn = 0;
		Globals::instance().getPortList().removePort(port,net::TCP);
	}
}

#include "server.moc"
