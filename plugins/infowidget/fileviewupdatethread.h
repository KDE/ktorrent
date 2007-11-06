/***************************************************************************
 *   Copyright (C) 2007 by Jaak Ristioja                                   *
 *   Ristioja@gmail.com                                                    *
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
#ifndef KTFILEVIEWUPDATETHREAD_H
#define KTFILEVIEWUPDATETHREAD_H

#include <qthread.h>

namespace kt
{
	class FileView;
	class TorrentInterface;
	
	class FileViewUpdateThread : public QThread
	{
		public:
			FileViewUpdateThread(FileView* fileview);
			~FileViewUpdateThread();
			
			void start(kt::TorrentInterface* new_tc, QThread::Priority priority);
			void run();
			
			/**
				Stops the running thread. It will do this in the background so when
				returning from this function, the thread might not be completely
				stopped yet. This is done as a safety measure to prevent failure in
				arbitrary thread termination.
			*/
			void stop();
		
		private:
			bool fillFileTree();
		
		private:
			FileView* fileview;
			kt::TorrentInterface* new_tc;
			bool stopThread;
	};
};

#endif
