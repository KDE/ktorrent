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

#include "pipe.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <util/log.h>
#include <util/functions.h>
#include "net/socket.h"


namespace bt
{
#ifdef Q_WS_WIN
	int socketpair(int sockets[2])
	{
		if (!InitWindowsSocketsAPI())
			return -1;
		
		sockets[0] = sockets[1] = -1;
		
		net::Socket sock(true,4);
		if (!sock.bind("127.0.0.1",0,true))
			return -1;
		
		net::Address local_addr = sock.getSockName();
		net::Socket writer(true,4);
		writer.setBlocking(false);
		writer.connectTo(local_addr);
		
		net::Address dummy;
		sockets[1] = sock.accept(dummy);
		if (sockets[1] < 0)
			return -1;
		
		if (!writer.connectSuccesFull())
		{
			closesocket(sockets[1]);
			return -1;
		}
		
		sockets[0] = writer.take();
		Out(SYS_GEN|LOG_DEBUG) << "Created wakeup pipe" << endl;
		return 0;
	}
#endif
	
	Pipe::Pipe() : reader(-1),writer(-1)
	{
		int sockets[2];
#ifndef Q_WS_WIN
		if (socketpair(AF_UNIX,SOCK_STREAM,0,sockets) == 0)
		{
			reader = sockets[1];
			writer = sockets[0];
			fcntl(writer,F_SETFL,O_NONBLOCK);
			fcntl(reader,F_SETFL,O_NONBLOCK);
		}
#else
		if (socketpair(sockets) == 0)
		{
			reader = sockets[1];
			writer = sockets[0];
		}
#endif
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Cannot create wakeup pipe" << endl;
		}
	}

	Pipe::~Pipe()
	{
#ifndef Q_WS_WIN
		::close(reader);
		::close(writer);
#else
		::closesocket(reader);
		::closesocket(writer);
#endif
	}

	int Pipe::read(Uint8* buffer, int max_len)
	{
#ifndef Q_WS_WIN
		return ::read(reader,buffer,max_len);
#else
		return ::recv(reader,(char*)buffer,max_len,0);
#endif
	}

	int Pipe::write(const bt::Uint8* data, int len)
	{
#ifndef Q_WS_WIN
		return ::write(writer,data,len);
#else
		return ::send(writer,(char *)data,len,0);
#endif
	}

}

