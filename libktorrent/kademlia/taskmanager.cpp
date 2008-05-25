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
#include <util/log.h>
#include <torrent/globals.h>
#include "taskmanager.h"
#include "nodelookup.h"
#include "dht.h"

using namespace bt;

namespace dht
{
	typedef bt::PtrMap<Uint32,Task>::iterator TaskItr;

	TaskManager::TaskManager() : next_id(0)
	{
		tasks.setAutoDelete(true);
	}


	TaskManager::~TaskManager()
	{
		queued.setAutoDelete(true);
		tasks.clear();
	}

	
	void TaskManager::addTask(Task* task)
	{
		Uint32 id = next_id++;
		task->setTaskID(id);
		if (task->isQueued())
			queued.append(task);
		else
			tasks.insert(id,task);
	}
		
	void TaskManager::removeFinishedTasks(const DHT* dh_table)
	{
		QValueList<Uint32> rm;
		for (TaskItr i = tasks.begin();i != tasks.end();i++)
		{
			if (i->second->isFinished())
				rm.append(i->first);
		}
		
		for (QValueList<Uint32>::iterator i = rm.begin();i != rm.end();i++)
		{
			tasks.erase(*i);
		}
		
		while (dh_table->canStartTask() && queued.count() > 0)
		{
			Task* t = queued.first();
			queued.removeFirst();
			Out(SYS_DHT|LOG_NOTICE) << "DHT: starting queued task" << endl;
			t->start();
			tasks.insert(t->getTaskID(),t);
		}
	}

}
