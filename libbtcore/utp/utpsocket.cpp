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
	
	UTPSocket::UTPSocket() : conn(0),blocking(true),polled_for_reading(false),polled_for_writing(false)
	{
	}
	
	UTPSocket::UTPSocket(Connection* conn) : conn(conn),blocking(true),polled_for_reading(false),polled_for_writing(false)
	{
		UTPServer & srv = bt::Globals::instance().getUTPServer();
		srv.attach(this,conn);
		setRemoteAddress(conn->remoteAddress());
		m_state = CONNECTED;
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
			try
			{
				conn->close();
			}
			catch (Connection::TransmissionError)
			{
				reset();
			}
		}
	}

	bool UTPSocket::connectSuccesFull()
	{
		bool ret = conn->connectionState() == CS_CONNECTED;
		if (ret)
		{
			setRemoteAddress(conn->remoteAddress());
			m_state = CONNECTED;
		}
			
		return ret;
	}

	bool UTPSocket::connectTo(const net::Address& addr)
	{
		UTPServer & srv = bt::Globals::instance().getUTPServer();
		reset();
		
		conn = srv.connectTo(addr);
		if (!conn)
			return false;
		
		srv.attach(this,conn);
		m_state = CONNECTING;
		if (blocking)
		{
			bool ret = conn->waitUntilConnected();
			if (ret)
				m_state = CONNECTED;
		
			return ret;
		}
		
		return conn->connectionState() == CS_CONNECTED;
	}

	int UTPSocket::fd() const
	{
		return -1;
	}

	const net::Address& UTPSocket::getPeerName() const
	{
		if (remote_addr_override)
			return addr;
		else if (conn)
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
		return conn != 0 && conn->connectionState() != CS_CLOSED;
	}

	int UTPSocket::recv(bt::Uint8* buf, int max_len)
	{
		if (!conn || conn->connectionState() == CS_CLOSED)
			return 0;
		
		try
		{
			if (conn->bytesAvailable() == 0)
			{
				if (blocking)
				{
					if (conn->waitForData())
						return conn->recv(buf,max_len);
					else
						return 0; // connection should be closed now
				}
				else
					return -1; // No data ready and not blocking so return -1
			}
			else
				return conn->recv(buf,max_len);
		}
		catch (Connection::TransmissionError & err)
		{
			close();
			return -1;
		}
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
		
		try
		{
			return conn->send(buf,len);
		}
		catch (Connection::TransmissionError & err)
		{
			close();
			return -1;
		}
	}

	void UTPSocket::setBlocking(bool on)
	{
		blocking = on;
	}

	bool UTPSocket::setTOS(unsigned char type_of_service)
	{
		Q_UNUSED(type_of_service);
		return true;
	}

	void UTPSocket::prepare(net::Poll* p, net::Poll::Mode mode)
	{
		if (conn)
		{
			UTPServer & srv = bt::Globals::instance().getUTPServer();
			srv.preparePolling(p,mode,conn);
			if (mode == net::Poll::OUTPUT)
				polled_for_writing = true;
			else
				polled_for_reading = true;
		}
	}

	bool UTPSocket::ready(const net::Poll* p, net::Poll::Mode mode) const
	{
		Q_UNUSED(p);
		if (!conn)
			return false;
		
		if (mode == net::Poll::OUTPUT)
		{
			if (polled_for_writing) 
			{
				polled_for_writing = false;
				return conn->isWriteable();
			}
		}
		else
		{
			if (polled_for_reading)
			{
				polled_for_reading = false;
				return bytesAvailable() > 0 || conn->connectionState() == CS_CLOSED;
			}
		}
		
		return false;
	}

}