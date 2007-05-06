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
#include <klocale.h>
#include <qlabel.h>
#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include "chunkdownloadview.h"


using namespace bt;
using namespace kt;

namespace kt
{
	
	ChunkDownloadViewItem::ChunkDownloadViewItem(KListView* cdv,kt::ChunkDownloadInterface* cd)
		: KListViewItem(cdv),cd(cd)
	{
		update();
	}
	
	void ChunkDownloadViewItem::update()
	{
		ChunkDownloadInterface::Stats s;
		cd->getStats(s);
			
		setText(0,QString::number(s.chunk_index));
		setText(1,QString("%1 / %2").arg(s.pieces_downloaded).arg(s.total_pieces));
		setText(2,s.current_peer_id);
		setText(3,KBytesPerSecToString(s.download_speed / 1024.0));
		setText(4,QString::number(s.num_downloaders));
	}
	
	int ChunkDownloadViewItem::compare(QListViewItem * i,int col,bool) const
	{
		ChunkDownloadViewItem* it = (ChunkDownloadViewItem*)i;
		kt::ChunkDownloadInterface* ocd = it->cd;
		ChunkDownloadInterface::Stats s;
		cd->getStats(s);
		ChunkDownloadInterface::Stats os;
		ocd->getStats(os);
		switch (col)
		{
			case 0: return CompareVal(s.chunk_index,os.chunk_index);
			case 1: return CompareVal(s.pieces_downloaded,os.pieces_downloaded);
			case 2: return QString::compare(s.current_peer_id,os.current_peer_id);
			case 3: return CompareVal(s.download_speed,os.download_speed);
			case 4: return CompareVal(s.num_downloaders,os.num_downloaders);
		}
		return 0;
	}
	
	
	ChunkDownloadView::ChunkDownloadView(QWidget *parent, const char *name)
	: ChunkDownloadViewBase(parent, name)
	{
		m_list_view->setShowSortIndicator(true);
		m_list_view->setAllColumnsShowFocus(true);
	
		m_list_view->setColumnAlignment(0,Qt::AlignLeft);
		m_list_view->setColumnAlignment(1,Qt::AlignCenter);
		m_list_view->setColumnAlignment(3,Qt::AlignRight);
		m_list_view->setColumnAlignment(4,Qt::AlignRight);
		curr_tc = 0;
	}
	
	
	ChunkDownloadView::~ChunkDownloadView()
	{}
	
	
	void ChunkDownloadView::addDownload(kt::ChunkDownloadInterface* cd)
	{
		ChunkDownloadViewItem* it = new ChunkDownloadViewItem(m_list_view,cd);
		items.insert(cd,it);
	}
		
	void ChunkDownloadView::removeDownload(kt::ChunkDownloadInterface* cd)
	{
		if (!items.contains(cd))
			return;
		
		ChunkDownloadViewItem* it = items[cd];
		delete it;
		items.remove(cd);
	}
	
	void ChunkDownloadView::removeAll()
	{
		m_list_view->clear();
		items.clear();
	}
	
	void ChunkDownloadView::changeTC(kt::TorrentInterface* tc)
	{
		curr_tc = tc;
		setEnabled(curr_tc != 0);
		update();
	}
	
	void ChunkDownloadView::update()
	{
		if (!curr_tc)
			return;
		
		QMap<ChunkDownloadInterface*,ChunkDownloadViewItem*>::iterator i = items.begin();
		while (i != items.end())
		{
			ChunkDownloadViewItem* it = i.data();
			it->update();
			i++;
		}
		m_list_view->sort();
		
		const TorrentStats & s = curr_tc->getStats();
		m_chunks_downloading->setText(QString::number(s.num_chunks_downloading));
		m_chunks_downloaded->setText(QString::number(s.num_chunks_downloaded));
		m_total_chunks->setText(QString::number(s.total_chunks));
		m_excluded_chunks->setText(QString::number(s.num_chunks_excluded));
		m_chunks_left->setText(QString::number(s.num_chunks_left));
		
		if( s.chunk_size / 1024 < 1024 )
			m_size_chunks->setText(QString::number(s.chunk_size / 1024) + "." + QString::number((s.chunk_size % 1024) / 100) + " KB");
		else
			m_size_chunks->setText(QString::number(s.chunk_size / 1024 / 1024) + "." + QString::number(((s.chunk_size / 1024) % 1024) / 100) + " MB");
	}
	
	void ChunkDownloadView::saveLayout(KConfig* cfg,const QString & group_name)
	{
		m_list_view->saveLayout(cfg,group_name);
	}
	
	void ChunkDownloadView::restoreLayout(KConfig* cfg,const QString & group_name)
	{
		m_list_view->restoreLayout(cfg,group_name);
	}
	
	void ChunkDownloadView::clear()
	{
		m_chunks_downloading->clear();
		m_chunks_downloaded->clear();
		m_total_chunks->clear();
		m_excluded_chunks->clear();
		m_size_chunks->clear();
	}
}

#include "chunkdownloadview.moc"
