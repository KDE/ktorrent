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
#ifndef BTDATACHECKERTHREAD_H
#define BTDATACHECKERTHREAD_H

#include <qthread.h>

namespace bt
{
	class Torrent;
	class DataChecker;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Thread which runs the data check.
	*/
	class DataCheckerThread : public QThread
	{
		DataChecker* dc;
		QString path;
		const Torrent & tor;
		QString dnddir;
		bool running;
		QString error;
	public:
		DataCheckerThread(DataChecker* dc,const QString & path,const Torrent & tor,const QString & dnddir);
		virtual ~DataCheckerThread();

		virtual void run();
		
		/// Get the data checker
		DataChecker* getDataChecker() {return dc;}
		
		/// Are we still running
		bool isRunning() const {return running;}
		
		/// Get the error (if any occured)
		QString getError() const {return error;}
	};

}

#endif
