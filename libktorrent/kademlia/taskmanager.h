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
#ifndef DHTTASKMANAGER_H
#define DHTTASKMANAGER_H

#include <qptrlist.h>
#include <util/ptrmap.h>
#include <util/constants.h>
#include "task.h"

namespace dht
{
	class DHT;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Manages all dht tasks.
	*/
	class TaskManager
	{
	public:
		TaskManager();
		virtual ~TaskManager();
		
		/**
		 * Add a task to manage.
		 * @param task 
		 */
		void addTask(Task* task);
				
		/**
		 * Remove all finished tasks.
		 * @param dh_table Needed to ask permission to start a task
		 */
		void removeFinishedTasks(const DHT* dh_table);
		
		/// Get the number of running tasks
		bt::Uint32 getNumTasks() const {return tasks.count();}
		
		/// Get the number of queued tasks
		bt::Uint32 getNumQueuedTasks() const {return queued.count();}

	private:
		bt::PtrMap<Uint32,Task> tasks;
		QPtrList<Task> queued;
		bt::Uint32 next_id;
	};

}

#endif
