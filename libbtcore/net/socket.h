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
#include <util/constants.h>
#include "address.h"

namespace net
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class BTCORE_EXPORT Socket
	{
	public:
		enum State
		{
			IDLE,
			CONNECTING,
			CONNECTED,
			BOUND,
			CLOSED
		};
	
		explicit Socket(int fd,int ip_version);
		explicit Socket(bool tcp,int ip_version);
		virtual ~Socket();
		
		void setNonBlocking();
		bool connectTo(const Address & addr);
		/// See if a connectTo was succesfull in non blocking mode
		bool connectSuccesFull();
		bool bind(const QString & ip,Uint16 port,bool also_listen);
		int send(const bt::Uint8* buf,int len);
		int recv(bt::Uint8* buf,int max_len);
		int sendTo(const bt::Uint8* buf,int size,const Address & addr);
		int recvFrom(bt::Uint8* buf,int max_size,Address & addr);
		int accept(Address & a);
		bool ok() const {return m_fd >= 0;}
		int fd() const {return m_fd;}
		bool setTOS(unsigned char type_of_service);
		const Address & getPeerName() const {return addr;}
		void close();
		State state() const {return m_state;}
		bool isIPv4() const {return m_ip_version == 4;}
		bool isIPv6() const {return m_ip_version == 6;}
		
		
		Uint32 bytesAvailable() const;
	private:
		void cacheAddress();
		
	private:
		int m_fd;
		int m_ip_version;
		State m_state;
		Address addr;
	};

}

#endif
