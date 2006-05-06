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
	
	Uint32 PeerViewItem::pvi_count = 0;
	// Global GeoIP pointer, gets destroyed when no PeerViewItem's exist
	static GeoIP* geo_ip = 0; 
	static QPixmap yes_pix;
	static QPixmap no_pix;
	static QPixmap lock_pix;
	static bool yes_no_pix_loaded = false;
	
		
	PeerViewItem::PeerViewItem(PeerView* pv,kt::PeerInterface* peer) : KListViewItem(pv),peer(peer)
	{	
		if (!yes_no_pix_loaded)
		{
			KIconLoader* iload = KGlobal::iconLoader();
			yes_pix = iload->loadIcon("button_ok",KIcon::Small);
			no_pix = iload->loadIcon("button_cancel",KIcon::Small);
			lock_pix = iload->loadIcon("ktencrypted",KIcon::Small);
			yes_no_pix_loaded = true;
		}
		
		pvi_count++;
		const char * hostname = 0;
		const char * country_code = 0;
		const char * country_name = 0;
		int country_id = 0;

		const PeerInterface::Stats & s = peer->getStats();
		hostname = s.ip_addresss.ascii();

		// open GeoIP if necessary
		if (!geo_ip)
			geo_ip = GeoIP_open(locate("data", "ktorrent/geoip/geoip.dat").ascii(),0);
		
		if (geo_ip)
		{
			country_id = GeoIP_id_by_name(geo_ip, hostname);
			country_code = GeoIP_country_code[country_id];
			country_name = GeoIP_country_name[country_id];
		}
		
		setText(0,s.ip_addresss);
		setText(2,s.client);
		setText(1, country_name);

		m_country = QString(country_name);
		
		QPixmap pix(locate("data", QString("ktorrent/geoip/%1.png").arg(QString(country_code)).lower()));
		setPixmap(1, pix);
		if (s.encrypted)
			setPixmap(0,lock_pix);
		update();
	}
	
	PeerViewItem::~PeerViewItem()
	{
		if (pvi_count > 0) // just to be sure, let this not wrap around
			pvi_count--;
		
		// destroy when not needed anymore
		if (pvi_count == 0 && geo_ip)
		{
			GeoIP_delete(geo_ip);
			geo_ip = 0;
		}
	}
	
	
	void PeerViewItem::update()
	{
		KLocale* loc = KGlobal::locale();
		const PeerInterface::Stats & s = peer->getStats();
		
		setText(3,KBytesPerSecToString(s.download_rate / 1024.0));
		setText(4,KBytesPerSecToString(s.upload_rate / 1024.0));
		//setPixmap(5,!s.choked ? yes_pix : no_pix);
		setText(5,s.choked ? i18n("Yes") : i18n("No"));
		//setPixmap(6,!s.snubbed ? yes_pix : no_pix);
		setText(6,s.snubbed ? i18n("Yes") : i18n("No"));
		setText(7,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));
		setPixmap(8,s.dht_support ? yes_pix : no_pix);
		setText(9,loc->formatNumber(s.aca_score,2));
		setPixmap(10,s.has_upload_slot ? yes_pix : QPixmap());
#undef SHOW_REQUESTS
#ifdef SHOW_REQUESTS
		setText(11,QString::number(s.num_requests));
#endif
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
			{
				int ret = QString::compare(m_country, pvi->m_country);
				return ret;
			}	
			case 2: return QString::compare(s.client,os.client);
			case 3: return CompareVal(s.download_rate,os.download_rate);
			case 4: return CompareVal(s.upload_rate,os.upload_rate);
			case 5: return CompareVal(s.choked,os.choked);
			case 6: return CompareVal(s.snubbed,os.snubbed);
			case 7: return CompareVal(s.perc_of_file,os.perc_of_file);
			case 8: return CompareVal(s.dht_support,os.dht_support);
			case 9: return CompareVal(s.aca_score,os.aca_score);
			case 10: return CompareVal(s.has_upload_slot,os.has_upload_slot);
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
		addColumn(i18n("Score"));
		addColumn(i18n("Upload Slot"));
#ifdef SHOW_REQUESTS
		addColumn(i18n("Requests"));
#endif
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
