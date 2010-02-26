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
			
			TimeStamp now = bt::Now();
			Uint32 num_ready = 0;
			SocketMonitor::Itr itr = sm->begin();
			while (itr != sm->end())
			{
				BufferedSocket* s = *itr;
				if (!s->socketDevice() || !s->socketDevice()->ok())
				{
					itr++;
					continue;
				}
				
				if (s->socketDevice()->ready(this,Poll::INPUT))
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
		sm->lock();
		
		reset();
		// Add the wake up pipe
		add(&wake_up);
	
		// fill the poll vector with all sockets
		SocketMonitor::Itr itr = sm->begin();
		while (itr != sm->end())
		{
			BufferedSocket* s = *itr;
			if (s && s->socketDevice()->ok())
			{
				s->socketDevice()->prepare(this,Poll::INPUT);
			}
			itr++;
		}
		sm->unlock();
		return poll();
	}
	
	void DownloadThread::wakeUp()
	{
		wake_up.wakeUp();
	}
}
