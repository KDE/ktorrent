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
#include "net/wakeuppipe.h"

namespace net
{

	WakeUpPipe::WakeUpPipe()
	{
		reader = new net::Socket(false,4);
		bt::Uint16 port = 50000;
		while (!reader->bind("127.0.0.1",port,false) && port - 50000 <= 10000)
		{
			port++;
		}
		
		writer = new net::Socket(false,4);
		writer->connectTo(net::Address("127.0.0.1",port));
	}


	WakeUpPipe::~WakeUpPipe()
	{
		delete reader;
		delete writer;
	}

	void WakeUpPipe::wakeUp()
	{
		char dummy[] = "dummy";
		writer->send((bt::Uint8*)dummy,5);
	}
		
	void WakeUpPipe::handleData()
	{
		bt::Uint8 buf[20];
		reader->recv(buf,20);
	}
}
