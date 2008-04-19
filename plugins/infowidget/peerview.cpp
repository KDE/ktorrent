/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson <joris.guisson@gmail.com>         *
 *   Copyright (C) 2007 by Modestas Vainius <modestas@vainius.eu>          *
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
#include <util/log.h>
#include "config.h"

#ifdef USE_SYSTEM_GEOIP
#include <GeoIP.h>
#else
#include "GeoIP.h"
#endif
#include "peerview.h"
#include "flagdb.h"

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
	static FlagDB flagDB(22, 18);
	static bool yes_no_pix_loaded = false;
	static bool geoip_db_exists = true;
	static QString geoip_data_file;
		
	PeerViewItem::PeerViewItem(PeerView* pv,kt::PeerInterface* peer) : KListViewItem(pv),peer(peer)
	{	
		if (!yes_no_pix_loaded)
		{
			KIconLoader* iload = KGlobal::iconLoader();
			/* Prefer builtin flag images to the ones provided by KDE */
			flagDB.addFlagSource("data",  QString("ktorrent/geoip/%1.png"));
			flagDB.addFlagSource("locale", QString("l10n/%1/flag.png"));
			yes_pix = iload->loadIcon("button_ok",KIcon::Small);
			no_pix = iload->loadIcon("button_cancel",KIcon::Small);
			lock_pix = iload->loadIcon("ktencrypted",KIcon::Small);
#ifdef USE_SYSTEM_GEOIP
			geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
			geoip_db_exists = (geo_ip != NULL);
#else
			geoip_db_exists = !locate("data", "ktorrent/geoip/geoip.dat").isNull(); 
			if(geoip_db_exists) {
				geoip_data_file = "ktorrent/geoip/geoip.dat";
			} else {
				geoip_db_exists = !locate("data", "ktorrent/geoip/GeoIP.dat").isNull();
				if (geoip_db_exists)
					geoip_data_file = "ktorrent/geoip/GeoIP.dat";
			}
#endif
			yes_no_pix_loaded = true;
		}
		
		pvi_count++;
		const char * hostname = 0;
		const char * country_code = 0;
		const char * country_name = 0;
		int country_id = 0;

		const PeerInterface::Stats & s = peer->getStats();
		hostname = s.ip_address.ascii();

		// open GeoIP if necessary
		if (!geo_ip && geoip_db_exists) {
#ifdef USE_SYSTEM_GEOIP
			geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
#else
			geo_ip = GeoIP_open(locate("data", geoip_data_file).ascii(),0);
#endif
		}
		if (geo_ip)
		{
			country_id = GeoIP_id_by_name(geo_ip, hostname);
			country_code = GeoIP_country_code[country_id];
			country_name = GeoIP_country_name[country_id];
			setText(1, country_name);
			m_country = QString(country_name);
		}
		else
		{
			setText(1,"N/A");
		}
		
	/*	if (s.fast_extensions)
			setText(0,s.ip_address + " (F)");
		else*/
			setText(0,s.ip_address);
			
		struct in_addr addr = {0};
		inet_aton(s.ip_address.ascii(),&addr);
		ip = ntohl(addr.s_addr);
			
		setText(2,s.client);
		
		if (country_code)
		{
			setPixmap(1, flagDB.getFlag(country_code));
		}
		
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

		if (s.download_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
			setText(3,KBytesPerSecToString(s.download_rate / 1024.0));
		else
			setText(3, "");
		if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
			setText(4,KBytesPerSecToString(s.upload_rate / 1024.0));
		else
			setText(4, "");
		//setPixmap(5,!s.choked ? yes_pix : no_pix);
		setText(5,s.choked ? i18n("Yes") : i18n("No"));
		//setPixmap(6,!s.snubbed ? yes_pix : no_pix);
		setText(6,s.snubbed ? i18n("Yes") : i18n("No"));
		setText(7,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));
		setPixmap(8,s.dht_support ? yes_pix : no_pix);
		setText(9,loc->formatNumber(s.aca_score,2));
		setPixmap(10,s.has_upload_slot ? yes_pix : QPixmap());
		setText(11,QString("%1 / %2").arg(s.num_down_requests).arg(s.num_up_requests));
		setText(12, BytesToString(s.bytes_downloaded));
		setText(13, BytesToString(s.bytes_uploaded));
	}
	
	int PeerViewItem::compare(QListViewItem * i,int col,bool) const
	{
		PeerViewItem* pvi = (PeerViewItem*) i;
		PeerInterface* op = pvi->peer;
		const PeerInterface::Stats & s = peer->getStats();
		const PeerInterface::Stats & os = op->getStats();
		switch (col)
		{
			case 0: return CompareVal(ip,pvi->ip); // use numeric representation to sort
			//return QString::compare(s.ip_address,os.ip_address);
			case 1:	return QString::compare(m_country, pvi->m_country);
			case 2: return QString::compare(s.client,os.client);
			case 3: return CompareVal(s.download_rate,os.download_rate);
			case 4: return CompareVal(s.upload_rate,os.upload_rate);
			case 5: return CompareVal(s.choked,os.choked);
			case 6: return CompareVal(s.snubbed,os.snubbed);
			case 7: return CompareVal(s.perc_of_file,os.perc_of_file);
			case 8: return CompareVal(s.dht_support,os.dht_support);
			case 9: return CompareVal(s.aca_score,os.aca_score);
			case 10: return CompareVal(s.has_upload_slot,os.has_upload_slot);
			case 11: return CompareVal(s.num_down_requests+s.num_up_requests, os.num_down_requests+os.num_up_requests);
			case 12: return CompareVal(s.bytes_downloaded, os.bytes_downloaded);
			case 13: return CompareVal(s.bytes_uploaded, os.bytes_uploaded);
			
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
		addColumn(i18n("Requests"));
		addColumn(i18n("Downloaded"));
		addColumn(i18n("Uploaded"));
		
		setAllColumnsShowFocus(true);
		setShowSortIndicator(true);
		
		setColumnAlignment(3,Qt::AlignRight);
		setColumnAlignment(4,Qt::AlignRight);
		setColumnAlignment(5,Qt::AlignCenter);
		setColumnAlignment(6,Qt::AlignCenter);
		setColumnAlignment(7,Qt::AlignRight);
		setColumnAlignment(8,Qt::AlignCenter);
		setColumnAlignment(9,Qt::AlignRight);
		setColumnAlignment(10,Qt::AlignCenter);
		setColumnAlignment(11,Qt::AlignCenter);
		setColumnAlignment(12,Qt::AlignRight);
		setColumnAlignment(13,Qt::AlignRight);
		
		for (Uint32 i = 0;i < (Uint32)columns();i++)
			setColumnWidthMode(i,QListView::Manual);
			
		setShowSortIndicator(true);
		
		menu = new KPopupMenu(this);
		kick_id = menu->insertItem(KGlobal::iconLoader()->loadIcon("delete_user", KIcon::NoGroup), i18n("to kick", "Kick peer"));
		ban_id = menu->insertItem(KGlobal::iconLoader()->loadIcon("filter",KIcon::NoGroup), i18n("to ban", "Ban peer"));
		
		connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
				this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));
		connect(menu, SIGNAL ( activated ( int ) ), this, SLOT ( contextItem ( int ) ) );
		setFrameShape(QFrame::NoFrame);
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

		PeerViewItem* pvi = it.data();
		if (pvi == curr)
			curr = 0;
				  
		delete pvi;
		items.erase(peer);
		
	}
	
	void PeerView::banPeer(kt::PeerInterface* peer)
	{
		if(!peer)
			return;
		
		IPBlocklist& filter = IPBlocklist::instance();
		const PeerInterface::Stats & s = peer->getStats();
		KNetwork::KIpAddress ip(s.ip_address);
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
	
	void PeerView::kickPeer(kt::PeerInterface* peer)
	{
		if(!peer)
			return;
		
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
			menu->setItemEnabled(kick_id, true);
			menu->popup(p);
		}
	}
	
	void PeerView::contextItem(int id)
	{
		if (id == ban_id && curr)
			banPeer(curr->getPeer());
		
		if (id == kick_id && curr)
			kickPeer(curr->getPeer());
	}
}
	
#include "peerview.moc"
