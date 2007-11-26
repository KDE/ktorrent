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
#include <QCloseEvent>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <util/error.h>
#include <util/log.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "scandlg.h"
#include "core.h"

using namespace bt;

namespace kt
{
	ScanDlg::ScanDlg(Core* core,bool auto_import,QWidget* parent) 
		: QDialog(parent),bt::DataCheckerListener(auto_import),mutex(QMutex::Recursive),core(core)
	{
		setupUi(this);
		m_cancel->setGuiItem(KStandardGuiItem::cancel());
		connect(m_cancel,SIGNAL(clicked()),this,SLOT(onCancelPressed()));
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		tc = 0;
		silently = false;
		restart = false;
		qm_controlled = false;
		scanning = false;
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		m_progress->setMaximum(100);
		m_progress->setValue(0);
	}
	
	ScanDlg::~ScanDlg()
	{
	}

	void ScanDlg::scan()
	{
		try
		{
			tc->startDataCheck(this);
			timer.start(500);
			scanning = true;
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(0,i18n("Error scanning data: %1",err.toString()));
		}
	}

	void ScanDlg::execute(bt::TorrentInterface* tc,bool silently)
	{
		m_torrent_label->setText(i18n("Scanning data of <b>%1</b> :",tc->getStats().torrent_name));
		adjustSize();
		m_cancel->setEnabled(true);
		this->silently = silently;
		this->tc = tc;
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		if (auto_import || tc->getStats().running)
			restart = true;
		
		qm_controlled = !tc->getStats().user_controlled;
		qm_priority = tc->getPriority();

		if (tc->getStats().running)
		{
			if (qm_controlled)
				core->getQueueManager()->stop(tc,true);
			else
				tc->stop(true);
		}
		
		scan();
	}

	void ScanDlg::progress(bt::Uint32 num,bt::Uint32 total)
	{
		QMutexLocker lock(&mutex);
		num_chunks = num;
		total_chunks = total;
	}
		 
	void ScanDlg::status(bt::Uint32 failed,bt::Uint32 downloaded)
	{
		QMutexLocker lock(&mutex);
		num_failed = failed;
		num_downloaded = downloaded;
	}
		
	void ScanDlg::finished()
	{
		QMutexLocker lock(&mutex);
		scanning = false;
		timer.stop();
		progress(100,100);
		update();
		if (!isStopped())
		{
			if (restart)
			{
				if (!qm_controlled)
					tc->start();
				else
				{
					tc->setPriority(qm_priority);
					core->getQueueManager()->orderQueue();
				}
			}
			
			if (silently)
				accept();
			else
			{
				// cancel now becomes a close button
				m_cancel->setGuiItem(KStandardGuiItem::close()); 
				disconnect(m_cancel,SIGNAL(clicked()),this,SLOT(onCancelPressed()));
				connect(m_cancel,SIGNAL(clicked()),this,SLOT(accept()));
			}
		}
		else
		{
			if (restart)
			{
				if (!qm_controlled)
					tc->start();
				else
				{
					tc->setPriority(qm_priority);
					core->getQueueManager()->orderQueue();
				}
			}
			
			QDialog::reject();
		}
	}
		 
	void ScanDlg::closeEvent(QCloseEvent* )
	{
		if (scanning)
			stop();
		else
			accept();
	}

	void ScanDlg::reject()
	{
		if (scanning)
			stop();
		else
		{
			QDialog::reject();
			deleteLater();
		}
	}

	void ScanDlg::accept()
	{
		QDialog::accept();
		deleteLater();
	}

	void ScanDlg::onCancelPressed()
	{
		stop();
	}

	void ScanDlg::update()
	{
		QMutexLocker lock(&mutex);
		m_progress->setMaximum(total_chunks);
		m_progress->setValue(num_chunks);
		m_chunks_found->setText(QString::number(num_downloaded));
		m_chunks_failed->setText(QString::number(num_failed));
	}
		
}

#include "scandlg.moc"

