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
#include "socketmonitor.h"
#include <math.h>
#include <unistd.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "bufferedsocket.h"
#include "uploadthread.h"
#include "downloadthread.h"

using namespace bt;

namespace net
{
	SocketMonitor SocketMonitor::self;

	SocketMonitor::SocketMonitor() : mutex(QMutex::Recursive),ut(0),dt(0),next_group_id(1)
	{
		dt = new DownloadThread(this);
		ut = new UploadThread(this);
	}


	SocketMonitor::~SocketMonitor()
	{
		shutdown();
	}
	
	void SocketMonitor::shutdown()
	{
		if (ut && ut->isRunning())
		{
			ut->stop();
			ut->signalDataReady(); // kick it in the nuts, if the thread is waiting for data
			if (!ut->wait(250))
			{
				ut->terminate();
				ut->wait();
			}
		}
		
		
		if (dt && dt->isRunning())
		{
			dt->stop();
			dt->wakeUp(); // wake it up if necessary
			if (!dt->wait(250))
			{
				dt->terminate();
				dt->wait();
			}
		}
		
		delete ut;
		delete dt;
		ut = 0;
		dt = 0;
	}

	
	void SocketMonitor::lock()
	{
		mutex.lock();
	}
	
	void SocketMonitor::unlock()
	{
		mutex.unlock();
	}
	
	void SocketMonitor::setDownloadCap(Uint32 bytes_per_sec)
	{
		DownloadThread::setCap(bytes_per_sec);
	}
	
	void SocketMonitor::setUploadCap(Uint32 bytes_per_sec)
	{
		UploadThread::setCap(bytes_per_sec);
	}
	
	void SocketMonitor::setSleepTime(Uint32 sleep_time)
	{
		DownloadThread::setSleepTime(sleep_time);
		UploadThread::setSleepTime(sleep_time);
	}
	
	void SocketMonitor::add(BufferedSocket* sock)
	{
		QMutexLocker lock(&mutex);
		
		bool start_threads = smap.size() == 0;
		smap.push_back(sock);
		
		if (start_threads)
		{
			Out(SYS_CON|LOG_DEBUG) << "Starting socketmonitor threads" << endl;
				
			if (!dt->isRunning())
				dt->start(QThread::IdlePriority);
			if (!ut->isRunning())
				ut->start(QThread::IdlePriority);
		}
		// wake up download thread so that it can start polling the new socket
		dt->wakeUp();
	}
	
	void SocketMonitor::remove(BufferedSocket* sock)
	{
		QMutexLocker lock(&mutex);
		if (smap.size() == 0)
			return;
		
		smap.remove(sock);
	}
	
	void SocketMonitor::signalPacketReady()
	{
		if (ut)
			ut->signalDataReady();
	}
	
	Uint32 SocketMonitor::newGroup(GroupType type,Uint32 limit,Uint32 assured_rate)
	{
		lock();
		Uint32 gid = next_group_id++;
		if (type == UPLOAD_GROUP)
			ut->addGroup(gid,limit,assured_rate);
		else
			dt->addGroup(gid,limit,assured_rate);
		unlock();
		return gid;
	}
		
	void SocketMonitor::setGroupLimit(GroupType type,Uint32 gid,Uint32 limit)
	{
		lock();
		if (type == UPLOAD_GROUP)
			ut->setGroupLimit(gid,limit);
		else
			dt->setGroupLimit(gid,limit);
		unlock();
	}
	
	void SocketMonitor::setGroupAssuredRate(GroupType type,Uint32 gid,Uint32 as)
	{
		lock();
		if (type == UPLOAD_GROUP)
			ut->setGroupAssuredRate(gid,as);
		else
			dt->setGroupAssuredRate(gid,as);
		unlock();
	}
		
	void SocketMonitor::removeGroup(GroupType type,Uint32 gid)
	{
		lock();
		if (type == UPLOAD_GROUP)
			ut->removeGroup(gid);
		else
			dt->removeGroup(gid);
		unlock();
	}

}
