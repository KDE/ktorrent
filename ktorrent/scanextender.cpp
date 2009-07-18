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
#include <KLocale>
#include <interfaces/torrentinterface.h>
#include "scanextender.h"

namespace kt
{
	
	ScanExtender::ScanExtender(bt::TorrentInterface* tc,QWidget* parent) 
	: QWidget(parent),bt::DataCheckerListener(false),tc(tc),done(false)
	{
		setupUi(this);
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
		
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		timer.start(500);
		
		cancel_button->setGuiItem(KStandardGuiItem::Cancel);
		close_button->setGuiItem(KStandardGuiItem::Close);
		close_button->setEnabled(false);
		connect(close_button,SIGNAL(clicked()),this,SIGNAL(closeRequested()));
		connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelPressed()));
		
		progress_bar->setFormat(i18n("Checked %v of %m chunks"));
		progress_bar->setValue(0);
		progress_bar->setMaximum(tc->getStats().total_chunks);
		
		QFont font = chunks_failed->font();
		font.setBold(true);
		chunks_failed->setFont(font);
		chunks_found->setFont(font);
		chunks_downloaded->setFont(font);
		chunks_not_downloaded->setFont(font);
	}

	ScanExtender::~ScanExtender()
	{
	}

	void ScanExtender::progress(bt::Uint32 num, bt::Uint32 total)
	{
		QMutexLocker lock(&mutex);
		num_chunks = num;
		total_chunks = total;
	}

	void ScanExtender::status(bt::Uint32 num_failed, bt::Uint32 num_found, bt::Uint32 num_downloaded, bt::Uint32 num_not_downloaded)
	{
		QMutexLocker lock(&mutex);
		this->num_downloaded = num_downloaded;
		this->num_failed = num_failed;
		this->num_found = num_found;
		this->num_not_downloaded = num_not_downloaded;
	}

	void ScanExtender::update()
	{
		QMutexLocker lock(&mutex);
		progress_bar->setMaximum(total_chunks);
		progress_bar->setValue(num_chunks);
		chunks_found->setText(QString::number(num_found));
		chunks_failed->setText(QString::number(num_failed));
		chunks_downloaded->setText(QString::number(num_downloaded));
		chunks_not_downloaded->setText(QString::number(num_not_downloaded));
	}

	void ScanExtender::finished()
	{
		timer.stop();
		update();
		progress_bar->setValue(total_chunks);
		progress_bar->setEnabled(false);
		cancel_button->setDisabled(true);
		close_button->setEnabled(true);
		done = true;
	}

	void ScanExtender::cancelPressed()
	{
		stop();
	}

	void ScanExtender::restart()
	{
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
		
		cancel_button->setEnabled(true);
		close_button->setEnabled(false);
		progress_bar->setEnabled(true);
		progress_bar->setFormat(i18n("Checked %v of %m chunks"));
		progress_bar->setValue(0);
		progress_bar->setMaximum(tc->getStats().total_chunks);
		tc->startDataCheck(this);
		timer.start(500);
	}
}

