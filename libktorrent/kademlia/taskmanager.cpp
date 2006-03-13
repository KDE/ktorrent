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
#include "taskmanager.h"

using namespace bt;

namespace dht
{

	TaskManager::TaskManager() : next_id(0)
	{
		tasks.setAutoDelete(true);
	}


	TaskManager::~TaskManager()
	{}

	
	void TaskManager::addTask(Task* task)
	{
		Uint32 id = next_id++;
		task->setTaskID(id);
		tasks.insert(id,task);
	}
		
	void TaskManager::removeTask(Task* task)
	{
		tasks.erase(task->getTaskID());
	}
		
	void TaskManager::removeFinishedTasks()
	{
		typedef bt::PtrMap<Uint32,Task>::iterator TaskItr;
		QValueList<Uint32> rm;
		for (TaskItr i = tasks.begin();i != tasks.end();i++)
			if (i->second->isFinished())
				rm.append(i->first);
		
		for (QValueList<Uint32>::iterator i = rm.begin();i != rm.end();i++)
			tasks.erase(*i);
	}

}
