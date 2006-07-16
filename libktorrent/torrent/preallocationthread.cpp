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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <util/log.h>
#include <util/error.h>
#include <qfile.h>
#include <klocale.h>
#include "preallocationthread.h"
#include "chunkmanager.h"
#include "globals.h"

#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

namespace bt
{

	PreallocationThread::PreallocationThread(ChunkManager* cman) : cman(cman),stopped(false),not_finished(false),done(false)
	{
		bytes_written = 0;
	}


	PreallocationThread::~PreallocationThread()
	{}

	void PreallocationThread::run()
	{
		try
		{
			cman->preallocateDiskSpace(this);
		}
		catch (Error & err)
		{
			setErrorMsg(err.toString());
		}
		
		mutex.lock();
		done = true;
		mutex.unlock();
		Out(SYS_GEN|LOG_NOTICE) << "PreallocationThread has finished" << endl;
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
	
	bool PreallocationThread::isDone() const
	{
		mutex.lock();
		bool tmp = done;
		mutex.unlock();
		return tmp;
	}
	
	bool PreallocationThread::isNotFinished() const
	{
		mutex.lock();
		bool tmp = not_finished;
		mutex.unlock();
		return tmp;
	}
	
	void PreallocationThread::setNotFinished()
	{
		mutex.lock();
		not_finished = true;
		mutex.unlock();
	}
}
