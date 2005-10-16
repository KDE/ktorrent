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
#include <interfaces/peerinterface.h>
#include "peerview.h"
#include "functions.h"

using namespace bt;
using namespace kt;

PeerViewItem::PeerViewItem(PeerView* pv,kt::PeerInterface* peer) : KListViewItem(pv),peer(peer)
{
	update();
}


void PeerViewItem::update()
{
	KLocale* loc = KGlobal::locale();
	PeerInterface::Stats s;
	peer->getStats(s);
	
	setText(0,s.ip_addresss);
	setText(1,s.client);
	setText(2,KBytesPerSecToString(s.download_rate / 1024.0));
	setText(3,KBytesPerSecToString(s.upload_rate / 1024.0));
	setText(4,s.choked ? i18n("yes") : i18n("no"));
	setText(5,s.snubbed ? i18n("yes") : i18n("no"));
	setText(6,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));
}

int PeerViewItem::compare(QListViewItem * i,int col,bool) const
{
	PeerInterface* op = ((PeerViewItem*)i)->peer;
	PeerInterface::Stats s;
	peer->getStats(s);
	PeerInterface::Stats os;
	op->getStats(os);
	switch (col)
	{
		case 0: return QString::compare(s.ip_addresss,os.ip_addresss);
		case 1: return QString::compare(s.client,os.client);
		case 2: return CompareVal(s.download_rate,os.download_rate);
		case 3: return CompareVal(s.upload_rate,os.upload_rate);
		case 4: return CompareVal(s.choked,os.choked);
		case 5: return CompareVal(s.snubbed,os.snubbed);
		case 6: return CompareVal(s.perc_of_file,os.perc_of_file);
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

void PeerView::addPeer(kt::PeerInterface* peer)
{
	PeerViewItem* i = new PeerViewItem(this,peer);
	items.insert(peer,i);
}

void PeerView::removePeer(kt::PeerInterface* peer)
{
	PeerViewItem* it = items[peer];
	delete it;
	items.erase(peer);
}

void PeerView::update()
{
	QMap<kt::PeerInterface*,PeerViewItem*>::iterator i = items.begin();
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
