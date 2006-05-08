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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <util/log.h>
#include <qfile.h>
#include <klocale.h>
#include "preallocationthread.h"
#include "torrentcontrol.h"
#include "globals.h"

#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

namespace bt
{

	PreallocationThread::PreallocationThread(TorrentControl* tc) : tc(tc),stopped(false)
	{
		bytes_written = 0;
	}


	PreallocationThread::~PreallocationThread()
	{}
	
	void PreallocationThread::addFile(const QString & path,Uint64 total_size)
	{
		mutex.lock();
		todo.insert(path,total_size);
		mutex.unlock();
	}
	
	bool PreallocationThread::expand(const QString & path,Uint64 max_size)
	{
		int fd = ::open(QFile::encodeName(path),O_WRONLY | O_LARGEFILE);
		if (ftruncate(fd,max_size) == -1)
		{
			setErrorMsg(i18n("Cannot preallocate diskspace : %1").arg(strerror(errno)));
			::close(fd);
			return false;
		}
		::close(fd);
		return true;
	}

	void PreallocationThread::run()
	{
		
		
		// start working on them
		while (!todo.empty() && !stopped)
		{
			QString path;
			Uint64 max_size = 0;
			
			// get the first from the map
			mutex.lock();
			QMap<QString,bt::Uint64>::iterator i = todo.begin();
			path = i.key();
			max_size = i.data();
			todo.erase(i);
			mutex.unlock();
			
			// expand it
			if (!expand(path,max_size))
				return;
		}
		
		Out() << "PreallocationThread::run finished" << endl;
	}
	
	void PreallocationThread::stop() 
	{
		mutex.lock();
		stopped = true;
		mutex.unlock();
	}
		
	void PreallocationThread::setErrorMsg(const QString & msg) 
	{
		mutex.lock();
		error_msg = msg; stopped = true;
		mutex.unlock();
	}
		
	bool PreallocationThread::isStopped() const 
	{
		mutex.lock();
		bool tmp = stopped;
		mutex.unlock();
		return tmp;
	}
		
	bool PreallocationThread::errorHappened() const 
	{
		mutex.lock();
		bool ret = !error_msg.isNull();
		mutex.unlock();
		return ret;
	}
		
	void PreallocationThread::written(Uint64 nb) 
	{
		mutex.lock();
		bytes_written += nb;
		mutex.unlock();
	}
		
	Uint64 PreallocationThread::bytesWritten()
	{
		mutex.lock();
		Uint64 tmp = bytes_written;
		mutex.unlock();
		return tmp;
	}
}
