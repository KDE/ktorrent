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
#include <util/log.h>
#include "preallocationthread.h"
#include "torrentcontrol.h"
#include "globals.h"

namespace bt
{

	PreallocationThread::PreallocationThread(TorrentControl* tc) : tc(tc),stopped(false)
	{
		bytes_written = 0;
	}


	PreallocationThread::~PreallocationThread()
	{}

	void PreallocationThread::run()
	{
		tc->preallocateDiskSpace(this);
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
