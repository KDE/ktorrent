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
#include "net/wakeuppipe.h"

using namespace bt;

namespace net
{

	WakeUpPipe::WakeUpPipe() : reader(-1),writer(-1)
	{
#ifndef Q_WS_WIN
		int sockets[2];
		if (socketpair(AF_UNIX,SOCK_STREAM,0,sockets) == 0)
		{
			reader = sockets[1];
			writer = sockets[0];
		}
#endif
	}


	WakeUpPipe::~WakeUpPipe()
	{
#ifndef Q_WS_WIN
		::close(reader);
		::close(writer);
#endif
	}

	void WakeUpPipe::wakeUp()
	{
#ifndef Q_WS_WIN
		char dummy[] = "dummy";
		int ret = write(writer,dummy,5);
		if (ret != 5)
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: wake up failed " << ret << endl;
#endif
	}
		
	void WakeUpPipe::handleData()
	{
#ifndef Q_WS_WIN
		bt::Uint8 buf[20];
		if (read(reader,buf,20) < 0)
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: read failed" << endl;
#endif
	}
}
