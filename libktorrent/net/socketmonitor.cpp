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
#include <unistd.h>
#include <qthread.h>
#include <qptrlist.h>
#include <sys/select.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "socketmonitor.h"
#include "bufferedsocket.h"

using namespace bt;

namespace net
{
	SocketMonitor SocketMonitor::self;
	Uint32 SocketMonitor::dcap = 0;
	Uint32 SocketMonitor::ucap = 0;
	
	class MonitorThread : public QThread
	{
		SocketMonitor* sm;
		bool running;
	public:
		MonitorThread(SocketMonitor* sm) : sm(sm),running(false)
		{
		}
		
		void stop()
		{
			running = false;
		}
		
		void run()
		{
			running = true;
			while (running)
				sm->update();
			running = false;
		}
		
		bool isRunning() const {return running;}
	};

	SocketMonitor::SocketMonitor() : mt(mt)
	{}


	SocketMonitor::~SocketMonitor()
	{
		if (mt && mt->isRunning())
		{
			mt->stop();
			if (!mt->wait(500))
			{
				mt->terminate();
				mt->wait();
			}
		}
		delete mt;
	}
	
	void SocketMonitor::setDownloadCap(Uint32 bytes_per_sec)
	{
		dcap = bytes_per_sec;
	}
	
	void SocketMonitor::setUploadCap(Uint32 bytes_per_sec)
	{
		ucap = bytes_per_sec;
	}
	
	void SocketMonitor::add(BufferedSocket* sock)
	{
		bool start_thread = false;
		mutex.lock();
		start_thread = smap.count() == 0 && (!mt || !mt->isRunning());
		smap.append(sock);
		mutex.unlock();
		
		if (start_thread)
		{
			Out(SYS_CON|LOG_DEBUG) << "Starting socketmonitor thread" << endl;
			last_selected = bt::GetCurrentTime();
			if (!mt)
				mt = new MonitorThread(this);
			
			mt->start();
		}
	}
	
	void SocketMonitor::remove(BufferedSocket* sock)
	{
		mutex.lock();
		smap.remove(sock);
		mutex.unlock();
		if (mt && smap.count() == 0 && mt->isRunning())
		{
			Out(SYS_CON|LOG_DEBUG) << "Stopping socketmonitor thread" << endl;
			mt->stop();
		}
	}
	
	void SocketMonitor::processIncomingData(QPtrList<BufferedSocket> & rbs,Uint32 now)
	{
		Uint32 allowance = (Uint32)round(dcap * (now - last_selected) * 0.001);
		Uint32 cnt = 0;
		QPtrList<BufferedSocket>::iterator i = rbs.begin();
		while (i != rbs.end() && allowance > 0)
		{
			Uint32 as = (Uint32)round(allowance / (rbs.count() - cnt));
			BufferedSocket* s = *i;
			Uint32 ret = s->readBuffered(as);
			if (ret > allowance)
				allowance = 0;
			else
				allowance -= ret;
			cnt++;
			i++;
		}
	}
	
	void SocketMonitor::processOutgoingData(QPtrList<BufferedSocket> & wbs,Uint32 now)
	{
		Uint32 allowance = (Uint32)round(ucap * (now - last_selected) * 0.001);
		Uint32 cnt = 0;
		QPtrList<BufferedSocket>::iterator i = wbs.begin();
		while (i != wbs.end() && allowance > 0)
		{
			Uint32 as = (Uint32)round(allowance / (wbs.count() - cnt));
			BufferedSocket* s = *i;
			Uint32 ret = s->writeBuffered(as);
/*			if (ret != as)
				bt::Out(SYS_GEN|LOG_DEBUG) << 
						fffffffff
						{}QString("as = %1 != ret = %2").arg(as).arg(ret) << endl;
	*/		
			if (ret > allowance)
				allowance = 0;
			else
				allowance -= ret;
			cnt++;
			i++;
		}
	}
	
	void SocketMonitor::update()
	{
	//	Out() << "SocketMonitor::update()" << endl;
		fd_set fds,wfds;
		FD_ZERO(&fds);
		FD_ZERO(&wfds);
		
		int max = 0;
		mutex.lock();
		QPtrList<BufferedSocket>::iterator itr = smap.begin();
		while (itr != smap.end())
		{
			BufferedSocket* s = *itr;
			if (s && s->ok())
			{
				// if we have bytes to write, see if we can write them
				if (s->bytesReadyToWrite())
					FD_SET(s->fd(),&wfds);
				
				FD_SET(s->fd(),&fds);
				
				if (s->fd() > max)
					max = s->fd();
			}
			itr++;
		}
		mutex.unlock();
		
		struct timeval tv = {0,50*1000};
		
		if (select(max+1,&fds,&wfds,NULL,&tv) > 0)
		{
			Uint32 now = bt::GetCurrentTime(); // get the current time
			Uint32 num_to_read = 0;
			QPtrList<BufferedSocket> rbs;
			QPtrList<BufferedSocket> wbs;
			
			mutex.lock();
			QPtrList<BufferedSocket>::iterator itr = smap.begin();
			while (itr != smap.end())
			{
				BufferedSocket* s = *itr;
				
				if (s->ok() && FD_ISSET(s->fd(),&fds))
				{
					// fd is set
					if (dcap == 0)
					{
						s->readBuffered(0);
					}
					else
					{
						num_to_read += s->bytesAvailable();
						rbs.append(s);
					}
				}
				
				if (s->ok() && FD_ISSET(s->fd(),&wfds))
				{
					if (ucap == 0)
					{
						// we can send bytes from the buffer so send them
						s->writeBuffered(0);
					}
					else
					{
						// add to the write queue
						wbs.append(s);
					}
				}
				itr++;
			}
			
			if (dcap > 0)
				processIncomingData(rbs,now);	
			
			if (ucap > 0)
				processOutgoingData(wbs,now);
			
			mutex.unlock();
			if (dcap > 0 || ucap > 0)
			{
				// sleep enough, so we don't consume to much
				if (last_selected - now < 50)
				{
					last_selected = now;
					usleep(100*1000);
				}	
			}
			last_selected = now;
			
		}
	}


}
