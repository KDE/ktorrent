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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qlabel.h>
#include <klocale.h>
#include <kprogress.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>
#include <util/error.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>

#include "scandialog.h"
#include "ktorrentcore.h"

using namespace bt;
using namespace kt;


	
ScanDialog::ScanDialog(KTorrentCore* core,bool auto_import,
					   QWidget* parent, const char* name, bool modal, WFlags fl)
	: ScanDlgBase(parent,name, modal,fl),DataCheckerListener(auto_import),mutex(true),core(core)
{
	m_cancel->setGuiItem(KStdGuiItem::cancel());
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
}

ScanDialog::~ScanDialog()
{
}

void ScanDialog::scan()
{
	try
	{
		tc->startDataCheck(this,auto_import);
		timer.start(500);
		scanning = true;
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,i18n("Error scanning data: %1").arg(err.toString()));
	}
	
}

void ScanDialog::execute(kt::TorrentInterface* tc,bool silently)
{
	m_torrent_label->setText(i18n("Scanning data of <b>%1</b> :").arg(tc->getStats().torrent_name));
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

void ScanDialog::finished()
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
			m_cancel->setGuiItem(KStdGuiItem::close()); 
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

void ScanDialog::progress(bt::Uint32 num,bt::Uint32 total)
{
	QMutexLocker lock(&mutex);
	num_chunks = num;
	total_chunks = total;
	
}

void ScanDialog::update()
{
	QMutexLocker lock(&mutex);
	m_progress->setTotalSteps(total_chunks);
	m_progress->setProgress(num_chunks);
	m_chunks_found->setText(QString::number(num_downloaded));
	m_chunks_failed->setText(QString::number(num_failed));
}

void ScanDialog::status(bt::Uint32 failed,bt::Uint32 downloaded)
{
	QMutexLocker lock(&mutex);
	num_failed = failed;
	num_downloaded = downloaded;
}

void ScanDialog::reject()
{
	if (scanning)
		stop();
	else
		QDialog::reject();
}

void ScanDialog::onCancelPressed()
{
	stop();
}

void ScanDialog::accept()
{
	QDialog::accept();
}

void ScanDialog::closeEvent(QCloseEvent* e)
{
	if (scanning)
		reject();
	else
		accept();
}

#include "scandialog.moc"

