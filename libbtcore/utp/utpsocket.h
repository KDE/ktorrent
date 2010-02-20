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

#ifndef UTP_UTPSOCKET_H
#define UTP_UTPSOCKET_H

#include <btcore_export.h>
#include <util/constants.h>
#include <net/socketdevice.h>

namespace utp
{
	class Connection;
	
	/**
		UTPSocket class serves as an interface for the networking code.
	*/
	class BTCORE_EXPORT UTPSocket : public net::SocketDevice
	{
	public:
		UTPSocket();
		UTPSocket(Connection* conn);
		virtual ~UTPSocket();
		
		virtual int fd() const;
		virtual bool ok() const;
		virtual int send(const bt::Uint8* buf,int len);
		virtual int recv(bt::Uint8* buf,int max_len);
		virtual void close();
		virtual void setBlocking(bool on);
		virtual bt::Uint32 bytesAvailable() const;
		virtual bool setTOS(unsigned char type_of_service);
		virtual bool connectTo(const net::Address & addr);
		virtual bool connectSuccesFull();
		virtual const net::Address & getPeerName() const;
		virtual net::Address getSockName() const;
		virtual void reset();
		virtual void prepare(net::Poll* p, net::Poll::Mode mode);
		virtual bool ready(const net::Poll* p, net::Poll::Mode mode) const;
		
	private:
		Connection* conn;
		bool blocking;
		mutable bool polled_for_reading;
		mutable bool polled_for_writing;
	};
}

#endif // UTPSOCKET_H
