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
		
using namespace bt;

namespace net
{
	Uint32 UploadThread::ucap = 0;

	UploadThread::UploadThread(SocketMonitor* sm) : sm(sm),running(false)
	{}


	UploadThread::~UploadThread()
	{}

	void UploadThread::run()
	{
		prev_upload_time = bt::Now();
		running = true;
		while (running)
			update();
	}
	
	void UploadThread::update()
	{
		sm->lock();
		bt::TimeStamp now = bt::Now();
		wbs.clear();
		Uint32 num_ready = 0;
		// loop over all sockets and see which ones have data ready
		SocketMonitor::Itr itr = sm->begin();
		while (itr != sm->end())
		{
			BufferedSocket* s = *itr;
			if (s && s->ok() && s->bytesReadyToWrite())
			{
				num_ready++;
				if (ucap == 0)
				{
					// we can send bytes from the buffer so send them
					s->writeBuffered(0,now);
				}
				else
				{
					// add to the write queue
					wbs.push_back(s);
				}
			}
			itr++;
		}
		
		if (ucap > 0 && wbs.size() > 0)
			processOutgoingData(now);
		else
			prev_upload_time = now;
		sm->unlock();
		
		if (num_ready == 0) // nobody was ready so go to sleep
			data_ready.wait(); 
		else
			msleep(1);
	}
	
	void UploadThread::signalDataReady()
	{
		data_ready.wakeOne();
	}
	
	void UploadThread::processOutgoingData(bt::TimeStamp now)
	{
		Uint32 allowance = (Uint32)ceil(ucap * (now - prev_upload_time) * 0.001);
		prev_upload_time = now;
		
		Uint32 bslot = allowance / wbs.size() + 1;
		
		Uint32 i = 0;
		Uint32 ns = wbs.size(); // num sockets
	
		// while we can send and there are sockets left to send
		while (ns > 0 && allowance > 0)
		{
			Uint32 as = bslot;
			if (as > allowance)
				as = allowance;
			
			BufferedSocket* s = wbs[i];
			if (s)
			{
				Uint32 ret = s->writeBuffered(as,now);
				// socket didn't sent everything it was allowed, so remove it from wbs
				if (ret != as)
				{
					wbs[i] = NULL;
					ns--;
				}
		
				if (ret > allowance)
					allowance = 0;
				else
					allowance -= ret;
			}
			i = (i + 1) % wbs.size();
		}
	}
}
