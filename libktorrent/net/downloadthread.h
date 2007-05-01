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
#ifndef NETDOWNLOADTHREAD_H
#define NETDOWNLOADTHREAD_H

#include <vector>
#include "networkthread.h"

struct pollfd;

namespace net
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Thread which processes incoming data
	 */
	class DownloadThread : public NetworkThread
	{
		static bt::Uint32 dcap;
		static bt::Uint32 sleep_time;
		
		std::vector<struct pollfd> fd_vec;
	
	public:
		DownloadThread(SocketMonitor* sm);
		virtual ~DownloadThread();
						  
	
		/// Set the download cap
		static void setCap(bt::Uint32 cap) {dcap = cap;}
		
		/// Set the sleep time when using download caps
		static void setSleepTime(bt::Uint32 stime);
	private:
		int fillPollVector();
		
		virtual void update();
		virtual bool doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now);

//		void processIncomingData(bt::TimeStamp now);
	};

}

#endif
