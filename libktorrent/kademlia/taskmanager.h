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
#ifndef DHTTASKMANAGER_H
#define DHTTASKMANAGER_H

#include <util/ptrmap.h>
#include <util/constants.h>
#include "task.h"

namespace dht
{

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
		 * Remove a task, also deletes it.
		 * @param task The task
		 */
		void removeTask(Task* task);
		
		/**
		 * Remove all finished tasks.
		 */
		void removeFinishedTasks();
		
		/// Get the number of running tasks
		bt::Uint32 getNumTasks() const {return tasks.count();}

	private:
		bt::PtrMap<Uint32,Task> tasks;
		bt::Uint32 next_id;
	};

}

#endif
