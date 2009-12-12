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
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <util/log.h>
#include <util/functions.h>
#include "net/socket.h"
#include "net/wakeuppipe.h"

using namespace bt;

namespace net
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

		Address local_addr = sock.getSockName();
		net::Socket writer(true,4);
		writer.setNonBlocking();
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

	WakeUpPipe::WakeUpPipe() : reader(-1),writer(-1)
	{
		int sockets[2];
#ifndef Q_WS_WIN
		if (socketpair(AF_UNIX,SOCK_STREAM,0,sockets) == 0)
#else
		if (socketpair(sockets) == 0)
#endif
		{
			reader = sockets[1];
			writer = sockets[0];
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Cannot create wakeup pipe" << endl;
		}
	}


	WakeUpPipe::~WakeUpPipe()
	{
#ifndef Q_WS_WIN
		::close(reader);
		::close(writer);
#else
		::closesocket(reader);
		::closesocket(writer);
#endif
	}

	void WakeUpPipe::wakeUp()
	{
		char dummy[] = "dummy";
#ifndef Q_WS_WIN
		int ret = write(writer,dummy,5);
#else
		int ret = ::send(writer,dummy,5,0);
#endif
		if (ret != 5)
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: wake up failed " << ret << endl;

	}
		
	void WakeUpPipe::handleData()
	{
		bt::Uint8 buf[20];
#ifndef Q_WS_WIN
		if (read(reader,buf,20) < 0)
#else
		if (::recv(reader,(char*)buf,20,0) < 0)
#endif
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: read failed" << endl;
	}
}
