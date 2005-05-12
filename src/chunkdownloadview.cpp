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
#include <klocale.h>
#include <libtorrent/peer.h>
#include <libtorrent/chunkdownload.h>
#include "chunkdownloadview.h"

using namespace bt;


ChunkDownloadView::ChunkDownloadView(QWidget *parent, const char *name)
: KListView(parent, name)
{
	addColumn(i18n("Chunk"));
	addColumn(i18n("Progress"));
	addColumn(i18n("Peer"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Assigned peers"));
	connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
	timer.start(250);
}


ChunkDownloadView::~ChunkDownloadView()
{}


void ChunkDownloadView::addDownload(ChunkDownload* cd)
{
	KListViewItem* it = new KListViewItem(this);
	items.insert(cd,it);
	update(cd,it);
}
	
void ChunkDownloadView::removeDownload(ChunkDownload* cd)
{
	if (!items.contains(cd))
		return;
	
	KListViewItem* it = items[cd];
	delete it;
	items.remove(cd);
}

void ChunkDownloadView::removeAll()
{
	clear();
	items.clear();
}

void ChunkDownloadView::update(const ChunkDownload* cd,KListViewItem* it)
{
	QString peer_id,speed = "0 kB/sec";

	peer_id = cd->getCurrentPeerID();
	speed = QString("%1 kB/sec").arg(cd->getDownloadSpeed() / 1024.0);
	
		
	it->setText(0,QString::number(cd->getChunkIndex()));
	it->setText(1,QString("%1 / %2").arg(cd->piecesDownloaded()).arg(cd->totalPieces()));
	it->setText(2,peer_id);
	it->setText(3,speed);
	it->setText(4,QString::number(cd->getNumDownloaders()));
}

void ChunkDownloadView::update()
{
	QMap<ChunkDownload*,KListViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		update(i.key(),i.data());
		i++;
	}
}

#include "chunkdownloadview.moc"
