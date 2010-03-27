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
#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <btcore_export.h>
#include <net/socketdevice.h>
#include "address.h"

namespace net
{
	const int SEND_FAILURE = 0;
	const int SEND_WOULD_BLOCK = -1;
	
	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class BTCORE_EXPORT Socket : public SocketDevice
	{
	public:
		explicit Socket(int fd,int ip_version);
		explicit Socket(bool tcp,int ip_version);
		virtual ~Socket();
		
		virtual void setBlocking(bool on);
		virtual bool connectTo(const Address & addr);
		virtual bool connectSuccesFull();
		virtual void close();
		virtual Uint32 bytesAvailable() const;
		virtual int send(const bt::Uint8* buf,int len);
		virtual int recv(bt::Uint8* buf,int max_len);
		virtual bool ok() const {return m_fd >= 0;}
		virtual int fd() const {return m_fd;}
		virtual bool setTOS(unsigned char type_of_service);
		virtual const Address & getPeerName() const {return addr;}
		virtual Address getSockName() const;
		
		virtual void reset();
		virtual void prepare(Poll* p,Poll::Mode mode);
		virtual bool ready(const Poll* p,Poll::Mode mode) const;
		
		bool bind(const QString & ip,Uint16 port,bool also_listen);
		bool bind(const Address & addr,bool also_listen);
		int accept(Address & a);
		
		int sendTo(const bt::Uint8* buf,int size,const Address & addr);
		int recvFrom(bt::Uint8* buf,int max_size,Address & addr);
		
		bool isIPv4() const {return m_ip_version == 4;}
		bool isIPv6() const {return m_ip_version == 6;}

		

		/// Take the filedescriptor from the socket
		int take();
		
	private:
		void cacheAddress();
		
	private:
		int m_fd;
		int m_ip_version;
		int r_poll_index;
		int w_poll_index;
	};

}

#endif
