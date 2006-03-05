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
#ifndef BTPREALLOCATIONTHREAD_H
#define BTPREALLOCATIONTHREAD_H

#include <qstring.h>
#include <qthread.h>
#include <qmutex.h>
#include <util/constants.h>



namespace bt
{
	class TorrentControl;	

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Thread to preallocate diskspace
	*/
	class PreallocationThread : public QThread
	{
		TorrentControl* tc;
		bool stopped;
		QString error_msg;
		Uint64 bytes_written;
		mutable QMutex mutex;
	public:
		PreallocationThread(TorrentControl* tc);
		virtual ~PreallocationThread();

		virtual void run();
		
		/**
		 * Stop the thread. 
		 */
		void stop();
		
		/**
		 * Set an error message, also calls stop
		 * @param msg The message
		 */
		void setErrorMsg(const QString & msg);
		
		/// See if the thread has been stopped
		bool isStopped() const;
		
		/// Did an error occur during the preallocation ?
		bool errorHappened() const;
		
		/// Get the error_msg
		const QString & errorMessage() const {return error_msg;}
		
		/// nb Number of bytes have been written
		void written(Uint64 nb);
		
		/// Get the number of bytes written
		Uint64 bytesWritten();
	};

}

#endif
