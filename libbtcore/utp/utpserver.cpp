/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include "utpserver.h"
#include <util/constants.h>
#include "utpprotocol.h"

namespace utp
{
	UTPServer::UTPServer(QObject* parent) : QThread(parent),sock(0),running(false)
	{

	}

	UTPServer::~UTPServer()
	{
	}

	bool UTPServer::bind(const net::Address& addr)
	{
		sock = new net::Socket(false,addr.ipVersion());
		if (!sock->bind(addr,false))
			return false;
		
		return true;
	}

	void UTPServer::run()
	{
		running = true;
		fd_set fds;
		FD_ZERO(&fds);
		while (running)
		{
			int fd = sock->fd();
			FD_SET(fd,&fds);
			struct timeval tv = {0,500000};
			if (select(fd + 1,&fds,0,0,&tv) > 0)
			{
				handlePacket();
			}
		}
	}
	
	void UTPServer::handlePacket()
	{
		int ba = sock->bytesAvailable();
		QByteArray packet(ba,0);
		net::Address addr;
		if (sock->recvFrom((bt::Uint8*)packet.data(),ba,addr) > 0)
		{
			// discard packets which are to small
			if (ba < sizeof(utp::Header))
				return;
		}
	}

}
