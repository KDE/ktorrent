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
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <libutil/functions.h>
#include <libtorrent/torrent.h>
#include <libtorrent/torrentcontrol.h>
#include "ktorrentmonitor.h"
#include "infowidget.h"
#include "peerview.h"
#include "chunkdownloadview.h"
#include "functions.h"
#include "downloadedchunkbar.h"
#include "availabilitychunkbar.h"
#include "iwfiletreeitem.h"

using namespace bt;



InfoWidget::InfoWidget(QWidget* parent, const char* name, WFlags fl)
		: InfoWidgetBase(parent,name,fl)
{
	monitor = 0;
	curr_tc = 0;
	setEnabled(false);
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

	bt::Torrent & tor = const_cast<bt::Torrent &>(curr_tc->getTorrent());
	if (tor.isMultiFile())
	{
		IWFileTreeDirItem* root = new IWFileTreeDirItem(m_file_view,tor.getNameSuggestion());
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			bt::TorrentFile & file = tor.getFile(i);
			root->insert(file.getPath(),file);
		}
		root->setOpen(true);
		m_file_view->setRootIsDecorated(true);
	}
	else
	{
		m_file_view->setRootIsDecorated(false);
		KListViewItem* item = new KListViewItem(m_file_view,
						  tor.getNameSuggestion(),
						  BytesToString(tor.getFileLength()));

		QString name = tor.getNameSuggestion();
		item->setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}
}

void InfoWidget::changeTC(bt::TorrentControl* tc)
{
	if (tc == curr_tc)
		return;
	
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
	m_av_chunk_bar->setTC(tc);
	setEnabled(tc != 0);
	update();
}

void InfoWidget::update()
{
	if (!curr_tc)
		return;

	m_chunks_downloading->setText(QString::number(curr_tc->getNumChunksDownloading()));
	m_chunks_downloaded->setText(QString::number(curr_tc->getNumChunksDownloaded()));
	m_total_chunks->setText(QString::number(curr_tc->getTotalChunks()));
	m_excluded_chunks->setText(QString::number(curr_tc->getNumChunksExcluded()));
	m_chunk_bar->update();
	m_av_chunk_bar->update();
	m_peer_view->update();
	m_chunk_view->update();
}


#include "infowidget.moc"

