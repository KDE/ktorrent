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
#include <torrent/torrentcontrol.h>

#include "scandialog.h"

using namespace bt;
using namespace kt;


	
ScanDialog::ScanDialog(QWidget* parent, const char* name, bool modal, WFlags fl)
: ScanDlgBase(parent,name, modal,fl)
{
	m_ok->setGuiItem(KStdGuiItem::ok());
	m_ok->setDisabled(true);
	m_cancel->setGuiItem(KStdGuiItem::cancel());
	connect(m_ok,SIGNAL(clicked()),this,SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
	tc = 0;
	silently = false;
}

ScanDialog::~ScanDialog()
{
}

void ScanDialog::scan()
{
	try
	{
		tc->doDataCheck(this);
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,i18n("Error scanning data : %1").arg(err.toString()));
	}
	progress(100,100);
	if (!isStopped())
	{
		m_ok->setEnabled(true);
		m_cancel->setEnabled(false);
		if (silently)
			accept();
	}
	else
	{
		QDialog::reject();
	}
}

void ScanDialog::execute(kt::TorrentInterface* tc,bool silently)
{
	m_ok->setEnabled(false);
	m_cancel->setEnabled(true);
	this->silently = silently;
	this->tc = tc;
	QTimer::singleShot(250,this,SLOT(scan()));
	exec();
}

void ScanDialog::progress(bt::Uint32 num,bt::Uint32 total)
{
	m_progress->setTotalSteps(total);
	m_progress->setProgress(num);
}

void ScanDialog::status(bt::Uint32 num_failed,bt::Uint32 num_downloaded)
{
	m_chunks_found->setText(QString::number(num_downloaded));
	m_chunks_failed->setText(QString::number(num_failed));
}

void ScanDialog::reject()
{
	//QDialog::reject();
	stop();
}

void ScanDialog::accept()
{
	QDialog::accept();
}



#include "scandialog.moc"

