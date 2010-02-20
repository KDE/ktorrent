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

#ifndef NET_SOCKETDEVICE_H
#define NET_SOCKETDEVICE_H

#include <btcore_export.h>
#include <util/constants.h>
#include <net/poll.h>
#include <net/address.h>

namespace net
{
	class SocketDevice
	{
	public:
		SocketDevice();
		virtual ~SocketDevice();
		
		enum State
		{
			IDLE,
			CONNECTING,
			CONNECTED,
			BOUND,
			CLOSED
		};
		
		virtual int fd() const = 0;
		virtual bool ok() const = 0;
		virtual int send(const bt::Uint8* buf,int len) = 0;
		virtual int recv(bt::Uint8* buf,int max_len) = 0;
		virtual void close() = 0;
		virtual void setBlocking(bool on) = 0;
		virtual Uint32 bytesAvailable() const = 0;
		virtual bool setTOS(unsigned char type_of_service) = 0;
		virtual bool connectTo(const Address & addr) = 0;
		/// See if a connectTo was succesfull in non blocking mode
		virtual bool connectSuccesFull() = 0;
		virtual const Address & getPeerName() const = 0;
		virtual Address getSockName() const = 0;
		
		/**
		* Set the remote address, used by Socks to set the actual address.
		* @param addr The address
		*/
		void setRemoteAddress(const Address & a) {addr = a; remote_addr_override = true;}
		
		/// reset the socket (i.e. close it and create a new one)
		virtual void reset() = 0;
		
		State state() const {return m_state;}
		
		/// Prepare for polling
		virtual void prepare(Poll* p,Poll::Mode mode) = 0;
		
		/// Check if the socket is ready according to the poll
		virtual bool ready(const Poll* p,Poll::Mode mode) const = 0;
		
	protected:
		State m_state;
		Address addr;
		bool remote_addr_override;
	};

}

#endif // NET_SOCKETDEVICE_H
