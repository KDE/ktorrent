/***************************************************************************
 *   Copyright (C) 2009 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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
#ifndef KT_SCANLISTENER_H
#define KT_SCANLISTENER_H

#include <QMutex>
#include <QObject>
#include <datachecker/datacheckerlistener.h>
#include <interfaces/torrentinterface.h>

namespace kt 
{

	class Extender;

	
	/**
		DataCheckerListener for KT which keeps track of the progress of a datascan
	*/
	class ScanListener : public QObject,public bt::DataCheckerListener
	{
		Q_OBJECT
	public:
		ScanListener(bt::TorrentInterface* tc);
		virtual ~ScanListener();
		
		/// Update progress info, runs in scan thread
		virtual void progress(bt::Uint32 num,bt::Uint32 total);
		
		/// Update status info, runs in scan thread
		virtual void status(bt::Uint32 num_failed,bt::Uint32 num_found,bt::Uint32 num_downloaded,bt::Uint32 num_not_downloaded);
		
		/// Scan finished, runs in app thread
		virtual void finished();
		
		/// Reset the listener
		void restart();
		
		/// Is the scan finished ?
		bool isFinished() const {return done;}
		
		/// Emit the close requested signal
		void emitCloseRequested();
		
		/// Get the torrent 
		bt::TorrentInterface* torrent() {return tc;}
		
		/// Create an extender for the job
		Extender* createExtender();
	
	signals:
		void scanFinished();
		void restarted();
		void closeRequested(ScanListener* me);
		
	public:
		bt::Uint32 num_chunks;
		bt::Uint32 total_chunks;
		bt::Uint32 num_found;
		bt::Uint32 num_downloaded;
		bt::Uint32 num_not_downloaded;
		bt::Uint32 num_failed;
		QMutex mutex;
		
	private:
		bool done;
		bt::TorrentInterface* tc;
	};

}

#endif // KT_SCANLISTENER_H
