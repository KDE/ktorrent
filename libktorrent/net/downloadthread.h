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
#include <qthread.h>
#include <util/constants.h>

struct pollfd;

namespace net
{
	class SocketMonitor;
	class BufferedSocket;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Thread which processes incoming data
	 */
	class DownloadThread : public QThread
	{
		static bt::Uint32 dcap;
		static bt::Uint32 sleep_time;
		
		SocketMonitor* sm;
		bool running;
		bt::TimeStamp prev_download_time;
		std::vector<struct pollfd> fd_vec;
		std::vector<BufferedSocket*> rbs;
	public:
		DownloadThread(SocketMonitor* sm);
		virtual ~DownloadThread();

		/// run the thread
		void run();

		/// Stop before the next update
		void stop() {running = false;}
		
		/// Is the thread running
		bool isRunning() const {return running;}
		
		/// Set the download cap
		static void setCap(bt::Uint32 cap) {dcap = cap;}
		
		/// Set the sleep time when using download caps
		static void setSleepTime(bt::Uint32 stime);
	private:
		int fillPollVector();
		void update();
		void processIncomingData(bt::TimeStamp now);
	};

}

#endif
