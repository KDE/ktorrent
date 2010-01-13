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

namespace utp
{
	UTPSocket::UTPSocket() : conn(0)
	{
	}
	
	UTPSocket::~UTPSocket()
	{
	}
	
	
	bt::Uint32 UTPSocket::bytesAvailable() const
	{

	}

	void UTPSocket::close()
	{

	}

	bool UTPSocket::connectSuccesFull()
	{

	}

	bool UTPSocket::connectTo(const net::Address& addr)
	{

	}

	int UTPSocket::fd() const
	{
		return -1;
	}

	const net::Address& UTPSocket::getPeerName() const
	{

	}

	net::Address UTPSocket::getSockName() const
	{

	}

	bool UTPSocket::ok() const
	{
		return conn != 0;
	}

	int UTPSocket::recv(bt::Uint8* buf, int max_len)
	{

	}

	void UTPSocket::reset()
	{

	}

	int UTPSocket::send(const bt::Uint8* buf, int len)
	{

	}

	void UTPSocket::setNonBlocking()
	{

	}

	bool UTPSocket::setTOS(unsigned char type_of_service)
	{

	}

}