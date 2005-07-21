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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <klistview.h>
#include <qlabel.h>
#include <qtimer.h>
#include <libutil/functions.h>
#include <libtorrent/torrent.h>
#include <libtorrent/torrentcontrol.h>
#include "ktorrentmonitor.h"
#include "infowidget.h"
#include "peerview.h"
#include "chunkdownloadview.h"
#include "functions.h"
#include "chunkbar.h"

using namespace bt;

InfoWidget::InfoWidget(QWidget* parent, const char* name, WFlags fl)
		: InfoWidgetBase(parent,name,fl)
{
	monitor = 0;
	curr_tc = 0;
	setEnabled(false);
	t = new QTimer(this);
	connect(t,SIGNAL(timeout()),this,SLOT(updateInfo()));
}

InfoWidget::~InfoWidget()
{
	delete monitor;
}

void InfoWidget::fillFileTree()
{
	m_file_view->clear();

	if (!curr_tc)
		return;

	const bt::Torrent & tor = curr_tc->getTorrent();
	if (tor.isMultiFile())
	{
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			bt::Torrent::File file;
			tor.getFile(i,file);
			new KListViewItem(m_file_view,
				tor.getNameSuggestion() + bt::DirSeparator() + file.path,
				BytesToString(file.size));
		}
	}
	else
	{
		new KListViewItem(m_file_view,
						  tor.getNameSuggestion(),
						  BytesToString(tor.getFileLength()));
	}
}

void InfoWidget::changeTC(bt::TorrentControl* tc)
{
	if (tc == curr_tc)
		return;

	if (tc)
		t->start(1000);
	else
		t->stop();
	
	curr_tc = tc;
	if (monitor)
	{
		delete monitor;
		monitor = 0;
		m_peer_view->removeAll();
		m_chunk_view->removeAll();
	}

	if (tc)
		monitor = new KTorrentMonitor(curr_tc,m_peer_view,m_chunk_view);

	fillFileTree();
	m_chunk_bar->setTC(tc);
	setEnabled(tc != 0);
}

void InfoWidget::updateInfo()
{
	if (!curr_tc)
		return;

	m_chunks_downloading->setText(QString::number(curr_tc->getNumChunksDownloading()));
	m_chunks_downloaded->setText(QString::number(curr_tc->getNumChunksDownloaded()));
	m_total_chunks->setText(QString::number(curr_tc->getTotalChunks()));
	m_chunk_bar->update();
}


#include "infowidget.moc"

