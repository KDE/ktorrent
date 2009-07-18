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
#ifndef KT_SCANEXTENDER_H
#define KT_SCANEXTENDER_H

#include <QWidget>
#include <QMutex>
#include <QTimer>
#include <datachecker/datacheckerlistener.h>
#include "ui_scanextender.h"

namespace bt
{
	class TorrentInterface;
}


namespace kt 
{
	class ScanExtender : public QWidget,public Ui_ScanExtender,public bt::DataCheckerListener
	{
		Q_OBJECT
	public:
		ScanExtender(bt::TorrentInterface* tc,QWidget* parent);
		virtual ~ScanExtender();
		
		/// Update progress info, runs in scan thread
		virtual void progress(bt::Uint32 num,bt::Uint32 total);
		
		/// Update status info, runs in scan thread
		virtual void status(bt::Uint32 num_failed,bt::Uint32 num_found,bt::Uint32 num_downloaded,bt::Uint32 num_not_downloaded);
		
		/// Scan finished, runs in app thread
		virtual void finished();
		
		bt::TorrentInterface* torrent() {return tc;}
		
		/// Restart the scan
		void restart();
		
		/// Is the scan finished ?
		bool scanFinished() const {return done;}
		
	private slots:
		void update();
		void cancelPressed();
		
	signals:
		/// Emitted when the close button is pressed
		void closeRequested();
		
	private:
		QMutex mutex;
		QTimer timer;
		bt::Uint32 num_chunks;
		bt::Uint32 total_chunks;
		bt::Uint32 num_found;
		bt::Uint32 num_downloaded;
		bt::Uint32 num_not_downloaded;
		bt::Uint32 num_failed;
		bt::TorrentInterface* tc;
		bool done;
	};
}

#endif // KT_SCANEXTENDER_H
