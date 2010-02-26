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

#include "poll.h"
#include <util/log.h>

#ifndef Q_WS_WIN
#include <sys/poll.h>
#else
#include <util/win32.h>
#endif

using namespace bt;

namespace net
{
	
	Poll::Poll() : num_sockets(0)
	{

	}

	Poll::~Poll()
	{

	}

	int Poll::add(int fd, Poll::Mode mode)
	{
		if (fd_vec.size() <= num_sockets)
		{
			// expand pollfd vector if necessary
			struct pollfd pfd;
			pfd.fd = fd;
			pfd.revents = 0;
			pfd.events = mode == INPUT ? POLLIN : POLLOUT;
			fd_vec.push_back(pfd);
		}
		else
		{
			// use existing slot
			struct pollfd & pfd = fd_vec[num_sockets];
			pfd.fd = fd;
			pfd.revents = 0;
			pfd.events = mode == INPUT ? POLLIN : POLLOUT;
		}
		
		int ret = num_sockets;
		num_sockets++;
		return ret;
	}
	
	
	int Poll::add(PollClient* pc)
	{
		int idx = add(pc->fd(),INPUT);
		poll_clients.insert(idx,pc);
		return idx;
	}


	bool Poll::ready(int index, Poll::Mode mode) const
	{
		if (index < 0 || index >= num_sockets)
			return false;
		
		return fd_vec[index].revents & (mode == INPUT ? POLLIN : POLLOUT);
	}

	void Poll::reset()
	{
		num_sockets = 0;
	}

	int Poll::poll(int timeout)
	{
		if (num_sockets == 0)
			return 0;
		
		int ret = 0;
#ifndef Q_WS_WIN
		ret = ::poll(&fd_vec[0],num_sockets,timeout);
#else
		ret = ::mingw_poll(&fd_vec[0],num_sockets,timeout);
#endif
		
		bt::PtrMap<int,PollClient>::iterator itr = poll_clients.begin();
		while (itr != poll_clients.end())
		{
			if (ret > 0 && ready(itr->first,INPUT))
				itr->second->handleData();
			itr->second->reset();
			itr++;
		}
		
		poll_clients.clear();
		return ret;
	}
	
	

}

