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



#include <qwaitcondition.h> 
#include "networkthread.h"

namespace net
{
	class SocketMonitor;
	class BufferedSocket;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class UploadThread : public NetworkThread
	{
		static bt::Uint32 ucap;
		static bt::Uint32 sleep_time;
	
		QWaitCondition data_ready;
	public:
		UploadThread(SocketMonitor* sm);
		virtual ~UploadThread();	
		
		/// Wake up thread, data is ready to be sent
		void signalDataReady();
		
		/// Set the upload cap
		static void setCap(bt::Uint32 uc) {ucap = uc;}
		
		/// Set the sleep time when using upload caps
		static void setSleepTime(bt::Uint32 stime);
	private: 
		virtual void update();
		virtual bool doGroup(SocketGroup* g,Uint32 & allowance,bt::TimeStamp now);
	};

}

#endif
