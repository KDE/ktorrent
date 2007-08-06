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
#include <math.h>
#include <sys/poll.h>
#include <util/functions.h>
#include "socketgroup.h"
#include "downloadthread.h"
#include "socketmonitor.h"
#include "bufferedsocket.h"
		
using namespace bt;

namespace net
{
	Uint32 DownloadThread::dcap = 0;
	Uint32 DownloadThread::sleep_time = 3;

	DownloadThread::DownloadThread(SocketMonitor* sm) : NetworkThread(sm)
	{
	}


	DownloadThread::~DownloadThread()
	{}
	
	void DownloadThread::update()
	{
		sm->lock();
		int num = fillPollVector();
		sm->unlock();
	
		int timeout = 10;	
		if (poll(&fd_vec[0],num,timeout) > 0)
		{
			sm->lock();
			TimeStamp now = bt::Now();
			Uint32 num_ready = 0;
			SocketMonitor::Itr itr = sm->begin();
			while (itr != sm->end())
			{
				BufferedSocket* s = *itr;
				int pi = s->getPollIndex();
				if (pi >= 0 && s->ok() && fd_vec[pi].revents & POLLIN)
				{
					// add to the correct group
					Uint32 gid = s->downloadGroupID();
					SocketGroup* g = groups.find(gid);
					if (!g)
						g = groups.find(0);
					
					g->add(s);
					num_ready++;
				}
				itr++;
			}
		
			if (num_ready > 0)
				doGroups(num_ready,now,dcap);
			prev_run_time = now;
			sm->unlock();
		}
		
		if (dcap > 0 || groups.count() > 0)
			msleep(sleep_time);
	}

	int DownloadThread::fillPollVector()
	{
		TimeStamp ts = bt::Now();
		int i = 0;
		
		// fill the poll vector with all sockets
		SocketMonitor::Itr itr = sm->begin();
		while (itr != sm->end())
		{
			BufferedSocket* s = *itr;
			if (s && s->ok() && s->fd() > 0)
			{
				if (fd_vec.size() <= i)
				{
					// expand pollfd vector if necessary
					struct pollfd pfd;
					pfd.fd = s->fd();
					pfd.revents = 0;
					pfd.events = POLLIN;
					fd_vec.push_back(pfd);
				}
				else
				{
					// use existing slot
					struct pollfd & pfd = fd_vec[i];
					pfd.fd = s->fd();
					pfd.revents = 0;
					pfd.events = POLLIN;
				}
				s->setPollIndex(i);
				i++;
				s->updateSpeeds(ts);
			}
			else
			{
				s->setPollIndex(-1);
			}
			itr++;
		}
		
		return i;
	}
	
	void DownloadThread::setSleepTime(Uint32 stime)
	{
		if (stime >= 1 && stime <= 10)
			sleep_time = stime;
	}
	
	bool DownloadThread::doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now)
	{
		return g->download(allowance,now);
	}
}
