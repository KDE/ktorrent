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
#include <util/functions.h>
#include "uploadthread.h"
#include "socketmonitor.h"
#include "bufferedsocket.h"
#include "socketgroup.h"
		
using namespace bt;

namespace net
{
	Uint32 UploadThread::ucap = 0;
	Uint32 UploadThread::sleep_time = 3;
	
	UploadThread::UploadThread(SocketMonitor* sm) : NetworkThread(sm)
	{}


	UploadThread::~UploadThread()
	{}

	
	void UploadThread::update()
	{
		sm->lock();
		bt::TimeStamp now = bt::Now();
	
		Uint32 num_ready = 0;
		// loop over all sockets and see which ones have data ready
		SocketMonitor::Itr itr = sm->begin();
		while (itr != sm->end())
		{
			BufferedSocket* s = *itr;
			if (s && s->ok() && s->bytesReadyToWrite())
			{
				SocketGroup* g = groups.find(s->uploadGroupID());
				if (!g)
					g = groups.find(0);
				
				g->add(s);
				num_ready++;
			}
			itr++;
		}
		
		if (num_ready > 0)
			doGroups(num_ready,now,ucap);
		prev_run_time = now;
		sm->unlock();
		
		if (num_ready == 0) // nobody was ready so go to sleep
			data_ready.wait(); 
		else
			msleep(sleep_time);
	}
	
	void UploadThread::signalDataReady()
	{
		data_ready.wakeOne();
	}
	
	void UploadThread::setSleepTime(Uint32 stime)
	{
		if (stime >= 1 && stime <= 10)
			sleep_time = stime;
	}
	
	bool UploadThread::doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now)
	{
		return g->upload(allowance,now);
	}
}
