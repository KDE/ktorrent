/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef NET_POLL_H
#define NET_POLL_H

#include <util/constants.h>
#include <btcore_export.h>
#include <vector>
#include <util/ptrmap.h>

struct pollfd;

namespace net
{

	/**
		Client for a Poll
	*/
	class BTCORE_EXPORT PollClient
	{
	public:
		PollClient() {}
		virtual ~PollClient() {}
		
		/// Get the filedescriptor to poll
		virtual int fd() const = 0;
		
		/// Handle data
		virtual void handleData() = 0;
		
		/// Reset the client called after poll finishes
		virtual void reset() = 0;
	};
	
	/**
		Class which does polling of sockets
	*/
	class BTCORE_EXPORT Poll
	{
	public:
		Poll();
		virtual ~Poll();
		
		enum Mode
		{
			INPUT, OUTPUT
		};
		
		/// Add a file descriptor to the poll (returns the index of it)
		int add(int fd,Mode mode);
		
		/// Add a poll client
		int add(PollClient* pc);
		
		/// Poll all sockets
		int poll(int timeout = -1);
		
		/// Check if a socket at an index is read
		bool ready(int index,Mode mode) const;
		
		/// Reset the poll
		void reset();
		
	private:
		std::vector<struct pollfd> fd_vec;
		bt::Uint32 num_sockets;
		bt::PtrMap<int,PollClient> poll_clients;
	};

}

#endif // NET_POLL_H
