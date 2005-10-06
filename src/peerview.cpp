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
#include <kglobal.h>
#include <libtorrent/peer.h>
#include "peerview.h"
#include "functions.h"

using namespace bt;

PeerViewItem::PeerViewItem(PeerView* pv,bt::Peer* peer) : KListViewItem(pv),peer(peer)
{
	update();
}


void PeerViewItem::update()
{
	KLocale* loc = KGlobal::locale();
	
	setText(0,peer->getIPAddresss());
	setText(1,peer->getPeerID().identifyClient());
	setText(2,KBytesPerSecToString(peer->getDownloadRate() / 1024.0));
	setText(3,KBytesPerSecToString(peer->getUploadRate() / 1024.0));
	setText(4,peer->isChoked() ? i18n("yes") : i18n("no"));
	setText(5,peer->isSnubbed() ? i18n("yes") : i18n("no"));
	setText(6,QString("%1 %").arg(loc->formatNumber(peer->percentAvailable(),2)));
}

int PeerViewItem::compare(QListViewItem * i,int col,bool) const
{
	Peer* op = ((PeerViewItem*)i)->peer;
	switch (col)
	{
		case 0:
			return QString::compare(peer->getIPAddresss(),op->getIPAddresss());
		case 1:
			return QString::compare(peer->getPeerID().identifyClient(),
									op->getPeerID().identifyClient());
		case 2: return CompareVal(peer->getDownloadRate(),op->getDownloadRate());
		case 3: return CompareVal(peer->getUploadRate(),op->getUploadRate());
		case 4: return CompareVal(peer->isChoked(),op->isChoked());
		case 5: return CompareVal(peer->isSnubbed(),op->isSnubbed());
		case 6: return CompareVal(peer->percentAvailable(),op->percentAvailable());
	}
	return 0;
}

PeerView::PeerView(QWidget *parent, const char *name)
		: KListView(parent, name)
{
	addColumn(i18n("IP"));
	addColumn(i18n("Client"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Up Speed"));
	addColumn(i18n("Choked"));
	addColumn(i18n("Snubbed"));
	addColumn(i18n("Availability"));
	setShowSortIndicator(true);
}


PeerView::~PeerView()
{}

void PeerView::addPeer(bt::Peer* peer)
{
	PeerViewItem* i = new PeerViewItem(this,peer);
	items.insert(peer,i);
}

void PeerView::removePeer(bt::Peer* peer)
{
	PeerViewItem* it = items[peer];
	delete it;
	items.erase(peer);
}

void PeerView::update()
{
	QMap<bt::Peer*,PeerViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		PeerViewItem* it = i.data();
		it->update();
		i++;
	}
}

void PeerView::removeAll()
{
	items.clear();
	clear();
}
	
#include "peerview.moc"
