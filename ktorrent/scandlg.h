/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef KT_SCANDLG_HH
#define KT_SCANDLG_HH

#include <QTimer>
#include <QMutex>
#include <KDialog>
#include <datachecker/datacheckerlistener.h>
#include "ui_scandlg.h"

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class Core;

	class ScanDlg : public KDialog,public bt::DataCheckerListener
	{
		Q_OBJECT
	public:
		ScanDlg(Core* core,bool auto_import,QWidget* parent);
		virtual ~ScanDlg();

		/// Starts the scan threadvent(QC
		void execute(bt::TorrentInterface* tc,bool silently);

	protected:
		/// Update progress info, runs in scan threadnted"))
		virtual void progress(bt::Uint32 num,bt::Uint32 total);
		 
		/// Update status info, runs in scan thread
		virtual void status(bt::Uint32 num_failed,bt::Uint32 num_found,bt::Uint32 num_downloaded,bt::Uint32 num_not_downloaded);
		
		/// Scan finished, runs in app thread
		virtual void finished();
		 
		/// Handle the close event
		virtual void closeEvent(QCloseEvent* e);

	protected slots:
		virtual void reject();
		virtual void accept();
		void onCancelPressed();
		/// Updates the GUI in app thread
		void update();
		void scan();

	private:
		bt::TorrentInterface* tc;
		QMutex mutex;
		QTimer timer;
		bt::Uint32 num_chunks;
		bt::Uint32 total_chunks;
		bt::Uint32 num_found;
		bt::Uint32 num_downloaded;
		bt::Uint32 num_not_downloaded;
		bt::Uint32 num_failed;
		bool silently;
		bool restart;
		bool scanning;
		Core* core;
		QProgressBar *m_progress;
		KPushButton *m_cancel;
		QLabel *m_torrent_label;
		QLabel *m_chunks_failed;
		QLabel *m_chunks_found;
		QLabel *m_chunks_downloaded;
		QLabel *m_chunks_not_downloaded;
	};
}

#endif
