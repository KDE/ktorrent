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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <libtorrent/peer.h>
#include <libtorrent/chunkdownload.h>
#include "chunkdownloadview.h"
#include "functions.h"

using namespace bt;

ChunkDownloadViewItem::ChunkDownloadViewItem(ChunkDownloadView* cdv,bt::ChunkDownload* cd)
	: KListViewItem(cdv),cd(cd)
{
	update();
}

void ChunkDownloadViewItem::update()
{
	QString peer_id,speed;

	peer_id = cd->getCurrentPeerID();
	speed = KBytesPerSecToString(cd->getDownloadSpeed() / 1024.0);
	
		
	setText(0,QString::number(cd->getChunkIndex()));
	setText(1,QString("%1 / %2").arg(cd->piecesDownloaded()).arg(cd->totalPieces()));
	setText(2,peer_id);
	setText(3,speed);
	setText(4,QString::number(cd->getNumDownloaders()));
}

int ChunkDownloadViewItem::compare(QListViewItem * i,int col,bool) const
{
	ChunkDownloadViewItem* it = (ChunkDownloadViewItem*)i;
	ChunkDownload* ocd = it->cd;
	switch (col)
	{
		case 0: return CompareVal(cd->getChunkIndex(),ocd->getChunkIndex());
		case 1: return CompareVal(cd->piecesDownloaded(),ocd->piecesDownloaded());
		case 2: return QString::compare(cd->getCurrentPeerID(),ocd->getCurrentPeerID());
		case 3: return CompareVal(cd->getDownloadSpeed(),ocd->getDownloadSpeed());
		case 4: return CompareVal(cd->getNumDownloaders(),ocd->getNumDownloaders());
	}
	return 0;
}


ChunkDownloadView::ChunkDownloadView(QWidget *parent, const char *name)
: KListView(parent, name)
{
	addColumn(i18n("Chunk"));
	addColumn(i18n("Progress"));
	addColumn(i18n("Peer"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Assigned Peers"));
	setShowSortIndicator(true);
}


ChunkDownloadView::~ChunkDownloadView()
{}


void ChunkDownloadView::addDownload(ChunkDownload* cd)
{
	ChunkDownloadViewItem* it = new ChunkDownloadViewItem(this,cd);
	items.insert(cd,it);
}
	
void ChunkDownloadView::removeDownload(ChunkDownload* cd)
{
	if (!items.contains(cd))
		return;
	
	ChunkDownloadViewItem* it = items[cd];
	delete it;
	items.remove(cd);
}

void ChunkDownloadView::removeAll()
{
	clear();
	items.clear();
}

void ChunkDownloadView::update()
{
	QMap<ChunkDownload*,ChunkDownloadViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		ChunkDownloadViewItem* it = i.data();
		it->update();
		i++;
	}
}

#include "chunkdownloadview.moc"
