/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include <sys/socket.h>
#include <netdb.h>
#include "reverseresolver.h"


namespace net
{
	ReverseResolverThread* ReverseResolver::worker = 0;
	
	ReverseResolver::ReverseResolver(QObject* parent): QObject(parent)
	{
	}
	
	
	ReverseResolver::~ReverseResolver()
	{
	}
	
	
	void ReverseResolver::resolveAsync(const net::Address& addr)
	{
		addr_to_resolve = addr;
		if (!worker)
		{
			worker = new ReverseResolverThread();
			worker->add(this);
			worker->start();
		}
		else
		{
			worker->add(this);
		}
	}

	QString ReverseResolver::resolve(const net::Address& addr)
	{
		if (!addr.address())
			return QString();
		
		char host[200];
		char service[200];
		memset(host,0,200);
		memset(service,0,200);
		if (getnameinfo(addr.address(),addr.length(),host,199,service,199,NI_NAMEREQD) == 0)
			return QString(host);
		else
			return QString();
	}
	
	
	void ReverseResolver::run()
	{
		QString res = resolve(addr_to_resolve);
		emit resolved(res);
	}
	
	
	void ReverseResolver::shutdown()
	{
		if (worker)
		{
			worker->stop();
			worker->wait();
			delete worker;
			worker = 0;
		}
	}

	
	ReverseResolverThread::ReverseResolverThread() : stopped(false)
	{
	}
	
	
	ReverseResolverThread::~ReverseResolverThread()
	{
	}
	
	void ReverseResolverThread::add(ReverseResolver* rr)
	{
		mutex.lock();
		todo_list.append(rr);
		mutex.unlock();
		more_data.wakeOne();
	}
	
	
	void ReverseResolverThread::run()
	{
		while (!stopped)
		{
			mutex.lock();
			if (!todo_list.empty())
			{
				ReverseResolver* rr = todo_list.first();
				todo_list.pop_front();
				mutex.unlock();
				rr->run();
				rr->deleteLater();
			}
			else
			{
				more_data.wait(&mutex);
				mutex.unlock();
			}
		}
		
		// cleanup if necessary
		foreach (ReverseResolver* rr,todo_list)
			rr->deleteLater();
		todo_list.clear();
	}
	
	void ReverseResolverThread::stop()
	{
		stopped = true;
		// wake up the thread if it is sleeping
		more_data.wakeOne();
	}


}

