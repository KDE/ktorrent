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
#ifndef NETNETWORKTHREAD_H
#define NETNETWORKTHREAD_H

#include <qthread.h>
#include <util/constants.h>
#include <util/ptrmap.h>
		
using bt::Uint32;

namespace net
{
	class SocketMonitor;
	class SocketGroup;
	class BufferedSocket;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Base class for the 2 networking threads. Handles the socket groups.
	*/
	class NetworkThread : public QThread
	{
	protected:
		SocketMonitor* sm;
		bool running;
		bt::PtrMap<Uint32,SocketGroup> groups;
		bt::TimeStamp prev_run_time;
		
	public:
		NetworkThread(SocketMonitor* sm);
		virtual ~NetworkThread();

		
		/**
		 * Add a new group with a given limit
		 * @param gid The group ID (cannot be 0, 0 is the default group)
		 * @param limit The limit in bytes per sec
		 */
		void addGroup(Uint32 gid,Uint32 limit);
		
		/**
		 * Remove a group 
		 * @param gid The group ID
		 */
		void removeGroup(Uint32 gid);
		
		/**
		 * Set the limit for a group
		 * @param gid The group ID 
		 * @param limit The limit 
		 */
		void setGroupLimit(Uint32 gid,Uint32 limit);
		
		/**
		 * The main function of the thread
		 */
		void run();
		
		/**
		 * Subclasses must implement this function
		 */
		virtual void update() = 0;
		
		/**
		 * Do one SocketGroup
		 * @param g The group
		 * @param allowance The groups allowance
		 * @param now The current time
		 * @return true if the group can go again
		 */
		virtual bool doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now) = 0;
		
		/// Stop before the next update
		void stop() {running = false;}
		
		/// Is the thread running
		bool isRunning() const {return running;}
		
	protected:
		/**
		 * Go over all groups and do them
		 * @param num_ready The number of ready sockets
		 * @param now The current time
		 * @param limit The global limit in bytes per sec
		 */
		void doGroups(Uint32 num_ready,bt::TimeStamp now,bt::Uint32 limit);
		
	private:
		Uint32 doGroupsLimited(Uint32 num_ready,bt::TimeStamp now,Uint32 & allowance);
	};

}

#endif
