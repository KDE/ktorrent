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
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ksocketaddress.h>
#include <qpoint.h>
#include <qlistview.h>
#include <kpopupmenu.h>
#include <interfaces/peerinterface.h>
#include <interfaces/functions.h>
#include <torrent/ipblocklist.h>
#include "peerview.h"
#include "GeoIP.h"

using namespace bt;
using namespace kt;

namespace kt
{
		
	PeerViewItem::PeerViewItem(PeerView* pv,kt::PeerInterface* peer) : KListViewItem(pv),peer(peer)
	{	
		const char * hostname;
		const char * country_code = 0;
		const char * country_name;
		int country_id;

		GeoIP * gi;
			
		const PeerInterface::Stats & s = peer->getStats();
		hostname = s.ip_addresss.ascii();

		gi = GeoIP_open(locate("data", "ktorrent/geoip/geoip.dat").ascii(),0);
		
		country_id = GeoIP_id_by_name(gi, hostname);
		country_code = GeoIP_country_code[country_id];
		country_name = GeoIP_country_name[country_id];
		
		if(gi)
		GeoIP_delete(gi);
		
		setText(0,s.ip_addresss);
		setText(2,s.client);
		setText(1, country_name);

		m_country = QString(country_name);
		
		QPixmap pix(locate("data", QString("ktorrent/geoip/%1.png").arg(QString(country_code)).lower()));
		setPixmap(0, pix);
		update();
	}
	
	
	void PeerViewItem::update()
	{
		KLocale* loc = KGlobal::locale();
		const PeerInterface::Stats & s = peer->getStats();
		
		setText(3,KBytesPerSecToString(s.download_rate / 1024.0));
		setText(4,KBytesPerSecToString(s.upload_rate / 1024.0));
		setText(5,s.choked ? i18n("yes") : i18n("no"));
		setText(6,s.snubbed ? i18n("yes") : i18n("no"));
		setText(7,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));
		setText(8,s.dht_support ? i18n("yes") : i18n("no"));
	}
	
	int PeerViewItem::compare(QListViewItem * i,int col,bool) const
	{
		PeerViewItem* pvi = (PeerViewItem*) i;
		PeerInterface* op = pvi->peer;
		const PeerInterface::Stats & s = peer->getStats();
		const PeerInterface::Stats & os = op->getStats();
		switch (col)
		{
			case 0: return QString::compare(s.ip_addresss,os.ip_addresss);
			case 1:	
				int ret = QString::compare(m_country, pvi->m_country);
				return ret;
				
			case 2: return QString::compare(s.client,os.client);
			case 3: return CompareVal(s.download_rate,os.download_rate);
			case 4: return CompareVal(s.upload_rate,os.upload_rate);
			case 5: return CompareVal(s.choked,os.choked);
			case 6: return CompareVal(s.snubbed,os.snubbed);
			case 7: return CompareVal(s.perc_of_file,os.perc_of_file);
			case 8: return CompareVal(s.dht_support,os.dht_support);
		}
		return 0;
	}
	
	PeerView::PeerView(QWidget *parent, const char *name)
			: KListView(parent, name)
	{
		addColumn(i18n("IP"));
		addColumn(i18n("Country"));
		addColumn(i18n("Client"));
		addColumn(i18n("Down Speed"));
		addColumn(i18n("Up Speed"));
		addColumn(i18n("Choked"));
		addColumn(i18n("Snubbed"));
		addColumn(i18n("Availability"));
		addColumn(i18n("DHT"));
		setShowSortIndicator(true);
		
		menu = new KPopupMenu(this);
		ban_id = menu->insertItem(KGlobal::iconLoader()->loadIcon("filter",KIcon::NoGroup), i18n("to ban", "Ban Peer"));
		
		connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
				this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));
		connect(menu, SIGNAL ( activated ( int ) ), this, SLOT ( contextItem ( int ) ) );
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
		QMap<kt::PeerInterface*,PeerViewItem*>::iterator it = items.find(peer);
		if (it == items.end())
		{
			return;
		}

		delete *it;
		items.erase(peer);
	}
	
	void PeerView::banPeer(kt::PeerInterface* peer)
	{
		if(!peer)
			return;
		
		IPBlocklist& filter = IPBlocklist::instance();
		const PeerInterface::Stats & s = peer->getStats();
		KNetwork::KIpAddress ip(s.ip_addresss);
		QString ips = ip.toString();
		/**
		 * @TODO Clean this up.
		 * this whole mess was because of KNetwork classes
		 * since we no longer use them, may I clean it up?
		 * I'll wait some time just in case...
		 **/
		if(ips.startsWith(":"))  
			filter.insert(ips.section(":",-1),3);
		else
			filter.insert(ips,3);
		peer->kill();
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
		sort();
	}
	
	void PeerView::removeAll()
	{
		items.clear();
		clear();
	}
	
	void PeerView::showContextMenu( KListView*, QListViewItem* item, const QPoint& p)
	{
		if(!item)
			return;
		
		curr = dynamic_cast<PeerViewItem*>(item);
		if (curr)
		{
			menu->setItemEnabled(ban_id, true);
			menu->popup(p);
		}
	}
	
	void PeerView::contextItem( int id )
	{
		if (id == ban_id && curr)
			banPeer(curr->getPeer());
	}
}
	
#include "peerview.moc"
