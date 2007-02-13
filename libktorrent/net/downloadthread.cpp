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
#include "downloadthread.h"
#include "socketmonitor.h"
#include "bufferedsocket.h"
		
using namespace bt;

namespace net
{
	Uint32 DownloadThread::dcap = 0;

	DownloadThread::DownloadThread(SocketMonitor* sm)
			: sm(sm),running(false)
	{}


	DownloadThread::~DownloadThread()
	{}
	
	void DownloadThread::run()
	{
		running = true;
		prev_download_time = bt::Now();
		while (running)
			update();
	}
	
	void DownloadThread::update()
	{
		sm->lock();
		int num = fillPollVector();
		sm->unlock();
	
		int timeout = 10;	
		if (poll(&fd_vec[0],num,timeout) > 0)
		{
			rbs.clear();
			sm->lock();
			TimeStamp now = bt::Now();
		
			SocketMonitor::Itr itr = sm->begin();
			while (itr != sm->end())
			{
				BufferedSocket* s = *itr;
				int pi = s->getPollIndex();
				if (pi >= 0 && s->ok() && fd_vec[pi].revents & POLLIN)
				{
					// data ready 
					if (dcap == 0)
					{
						s->readBuffered(0,now);
					}
					else
					{
						rbs.push_back(s);
					}
				}
				
				itr++;
			}
		
			if (dcap > 0 && rbs.size() > 0)
				processIncomingData(now);
			else
				prev_download_time = now;
			sm->unlock();
		}
		
		if (dcap > 0)
			msleep(1);
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
	
	void DownloadThread::processIncomingData(bt::TimeStamp now)
	{
		Uint32 allowance = (Uint32)ceil(1.02 * dcap * (now - prev_download_time) * 0.001);
		prev_download_time = now;
		
		Uint32 bslot = allowance / rbs.size() + 1;
		Uint32 i = 0;
		Uint32 ns = rbs.size(); // num sockets
	
		// while we can send and there are sockets left to send
		while (ns > 0 && allowance > 0)
		{
			Uint32 as = bslot;
			if (as > allowance)
				as = allowance;
			
			BufferedSocket* s = rbs[i];
			if (s)
			{
				Uint32 ret = s->readBuffered(as,now);
				// if this socket did what it was supposed to do, 
				// it can have another go if stuff is leftover
				// if it doesn't, we set it to NULL, so that it will not 
				// get it's turn again
				if (ret != as) 
				{
					rbs[i] = NULL;
					ns--;
				}
			
				if (ret > allowance)
					allowance = 0;
				else
					allowance -= ret;
			}

			i = (i + 1) % rbs.size();
		}
	}

}
