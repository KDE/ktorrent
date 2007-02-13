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
#ifndef NETUPLOADTHREAD_H
#define NETUPLOADTHREAD_H

#include <qthread.h>
#include <qwaitcondition.h> 
#include <vector>
#include <util/constants.h>

namespace net
{
	class SocketMonitor;
	class BufferedSocket;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class UploadThread : public QThread
	{
		static bt::Uint32 ucap;
		
		SocketMonitor* sm;
		bool running;
		bt::TimeStamp prev_upload_time;
		std::vector<BufferedSocket*> wbs;
	
		QWaitCondition data_ready;
	public:
		UploadThread(SocketMonitor* sm);
		virtual ~UploadThread();

		/// run the thread
		void run();

		/// Stop before the next update
		void stop() {running = false;}
		
		/// Is the thread running
		bool isRunning() const {return running;}
		
		/// Wake up thread, data is ready to be sent
		void signalDataReady();
		
		/// Set the upload cap
		static void setCap(bt::Uint32 uc) {ucap = uc;}
	private: 
		void update();
		void processOutgoingData(bt::TimeStamp now);
	};

}

#endif
