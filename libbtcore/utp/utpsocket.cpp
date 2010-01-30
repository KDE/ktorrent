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

#include "utpsocket.h"
#include "connection.h"
#include <torrent/globals.h>
#include "utpserver.h"

namespace utp
{
	UTPSocket::UTPSocket() : conn(0),blocking(true)
	{
	}
	
	UTPSocket::UTPSocket(Connection* conn) : conn(conn),blocking(true)
	{
		UTPServer & srv = bt::Globals::instance().getUTPServer();
		srv.attach(this,conn);
	}

	UTPSocket::~UTPSocket()
	{
		close();
		reset();
	}
	
	
	bt::Uint32 UTPSocket::bytesAvailable() const
	{
		return conn->bytesAvailable();
	}

	void UTPSocket::close()
	{
		if (conn)
		{
			conn->close();
		}
	}

	bool UTPSocket::connectSuccesFull()
	{
		return conn->connectionState() == CS_CONNECTED;
	}

	bool UTPSocket::connectTo(const net::Address& addr)
	{
		UTPServer & srv = bt::Globals::instance().getUTPServer();
		reset();
		
		conn = srv.connectTo(addr);
		srv.attach(this,conn);
		if (blocking)
			return conn->waitUntilConnected();
		
		return true;
	}

	int UTPSocket::fd() const
	{
		return -1;
	}

	const net::Address& UTPSocket::getPeerName() const
	{
		if (conn)
			return conn->remoteAddress();
		else
			return net::Address::null;
	}

	net::Address UTPSocket::getSockName() const
	{
		return net::Address::null;
	}

	bool UTPSocket::ok() const
	{
		return conn != 0;
	}

	int UTPSocket::recv(bt::Uint8* buf, int max_len)
	{
		if (!conn || conn->connectionState() == CS_CLOSED)
			return -1;
		
		if (conn->bytesAvailable() == 0 && blocking)
		{
			if (conn->waitForData())
				return conn->recv(buf,max_len);
			else
				return 0; // connection should be closed now
		}
		else
			return conn->recv(buf,max_len);
	}

	void UTPSocket::reset()
	{
		if (conn)
		{
			UTPServer & srv = bt::Globals::instance().getUTPServer();
			srv.detach(this,conn);
			conn = 0;
		}
	}

	int UTPSocket::send(const bt::Uint8* buf, int len)
	{
		if (!conn)
			return -1;
		
		return conn->send(buf,len);
	}

	void UTPSocket::setBlocking(bool on)
	{
		blocking = on;
	}

	bool UTPSocket::setTOS(unsigned char type_of_service)
	{
		return false;
	}

	void UTPSocket::setRemoteAddress(const net::Address& a)
	{
		// TODO: implement this
	}

}