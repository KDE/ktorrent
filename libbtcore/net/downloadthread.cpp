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
#include "downloadthread.h"
#include <math.h>
#include <QtGlobal>
#ifndef Q_WS_WIN
#include <sys/poll.h>
#else
#include <util/win32.h>
#endif
#include <util/functions.h>
#include <util/log.h>
#include "socketgroup.h"
#include "socketmonitor.h"
#include "bufferedsocket.h"
#include "wakeuppipe.h"
		
using namespace bt;

namespace net
{
	Uint32 DownloadThread::dcap = 0;
	Uint32 DownloadThread::sleep_time = 50;

	DownloadThread::DownloadThread(SocketMonitor* sm) : NetworkThread(sm)
	{
	}


	DownloadThread::~DownloadThread()
	{
	}
	
	void DownloadThread::update()
	{
		if (waitForSocketReady() > 0)
		{
			bool group_limits = false;
			sm->lock();
			if (fd_vec[0].revents & POLLIN)
			{
				// wake up was triggered
				wake_up.handleData();
			}
			
			TimeStamp now = bt::Now();
			Uint32 num_ready = 0;
			SocketMonitor::Itr itr = sm->begin();
			while (itr != sm->end())
			{
				BufferedSocket* s = *itr;
				if (!s->socketDevice()->ok())
				{
					itr++;
					continue;
				}
				
				int pi = s->getPollIndex();
				bool ready = false;
				if (pi >= 0)
					ready = fd_vec[pi].revents & POLLIN;
				else
					ready = s->socketDevice()->bytesAvailable() > 0;
		
				if (ready)
				{
					// add to the correct group
					Uint32 gid = s->downloadGroupID();
					if (gid > 0)
						group_limits = true;
					
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
			sm->unlock();
			
			// to prevent huge CPU usage sleep a bit if we are limited (either by a global limit or a group limit)
			if (dcap > 0 || group_limits)
			{
				TimeStamp diff = now - prev_run_time;
				if (diff < sleep_time)
					msleep(sleep_time - diff);
			}
			prev_run_time = now;
		}
	}
	
	
	void DownloadThread::setSleepTime(Uint32 stime)
	{
		sleep_time = stime;
	}
	
	bool DownloadThread::doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now)
	{
		return g->download(allowance,now);
	}
	
	int DownloadThread::waitForSocketReady()
	{
		unsigned int i = 0;
		sm->lock();
		
		// Add the wake up pipe
		if (fd_vec.size() >= 1)
		{
			struct pollfd & wfd = fd_vec[0];
			wfd.fd = wake_up.readerSocket();
			wfd.revents = 0;
			wfd.events = POLLIN;
		}
		else
		{
			struct pollfd wfd;
			wfd.fd = wake_up.readerSocket();
			wfd.revents = 0;
			wfd.events = POLLIN;
			fd_vec.push_back(wfd);
		}
		i++;
		
		// fill the poll vector with all sockets
		SocketMonitor::Itr itr = sm->begin();
		while (itr != sm->end())
		{
			BufferedSocket* s = *itr;
			if (s && s->socketDevice()->ok())
			{
				if (fd_vec.size() <= i)
				{
					// expand pollfd vector if necessary
					struct pollfd pfd;
					pfd.fd = s->socketDevice()->fd();
					pfd.revents = 0;
					pfd.events = POLLIN;
					fd_vec.push_back(pfd);
				}
				else
				{
					// use existing slot
					struct pollfd & pfd = fd_vec[i];
					pfd.fd = s->socketDevice()->fd();
					pfd.revents = 0;
					pfd.events = POLLIN;
				}
				s->setPollIndex(i);
				i++;
			}
			else
			{
				s->setPollIndex(-1);
			}
			itr++;
		}
		sm->unlock();
	
#ifndef Q_WS_WIN
		return poll(&fd_vec[0],i,-1);
#else
		return mingw_poll(&fd_vec[0],i,-1);
#endif
	}
	
	void DownloadThread::wakeUp()
	{
		wake_up.wakeUp();
	}
}
