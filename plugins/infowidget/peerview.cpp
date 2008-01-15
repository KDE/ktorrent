/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "peerview.h"

#include <QHeaderView>
#include <klocale.h>
#include <kicon.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <interfaces/peerinterface.h>
#include <torrent/ipblocklist.h>
#include <util/functions.h>
#include "flagdb.h"

#ifdef USE_SYSTEM_GEOIP
#include <GeoIP.h>
#else
#include "GeoIP.h"
#endif

using namespace bt;

namespace kt
{
	static KIcon yes,no;
	static bool icons_loaded = false;
	static GeoIP* geo_ip = 0;
	static FlagDB flagDB(22, 18);
	static bool geoip_db_exists = true;
	static QString geoip_data_file;
	
	PeerViewItem::PeerViewItem(PeerView* pv,PeerInterface* peer) : QTreeWidgetItem(pv,QTreeWidgetItem::UserType),peer(peer)
	{
		if (!icons_loaded)
		{
			yes = KIcon("dialog-ok");
			no = KIcon("dialog-cancel");
			icons_loaded = true;
			flagDB.addFlagSource("data",  QString("ktorrent/%1.png"));
			flagDB.addFlagSource("locale", QString("l10n/%1/flag.png"));
#ifdef USE_SYSTEM_GEOIP
			geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
			geoip_db_exists = (geo_ip != NULL);
#else
			geoip_db_exists = !KStandardDirs::locate("data", "ktorrent/geoip.dat").isNull();
			if(geoip_db_exists) 
			{
				geoip_data_file = "ktorrent/geoip.dat";
			} 
			else 
			{
				geoip_db_exists = !KStandardDirs::locate("data", "ktorrent/GeoIP.dat").isNull();
				if (geoip_db_exists)
					geoip_data_file = "ktorrent/GeoIP.dat";
			}
#endif
		}
		const PeerInterface::Stats & s = peer->getStats();
		
		// open GeoIP if necessaryt
		if (!geo_ip && geoip_db_exists) 
		{
#ifdef USE_SYSTEM_GEOIP
			geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
#else
			geo_ip = GeoIP_open(KStandardDirs::locate("data", geoip_data_file).toLocal8Bit(),0);
#endif
		}
		
		if (geo_ip)
		{
			int country_id = GeoIP_id_by_name(geo_ip, s.ip_address.toAscii());
			setText(1, GeoIP_country_name[country_id]);
			setIcon(1, flagDB.getFlag(GeoIP_country_code[country_id]));
		} 
		else
		{
			setText(1,"N/A");
		}	
		
		// stuff that doesn't change
		setText(0,s.ip_address);
		setText(2,s.client);
		setIcon(8,s.dht_support ? yes : no);
		if (s.encrypted)
			setIcon(0,KIcon("ktencrypted"));

		// initialize the stats
		stats = s;
		update(true);
	}

	PeerViewItem::~PeerViewItem()
	{
	}
	
	bool PeerViewItem::operator < (const QTreeWidgetItem & other) const
	{
		const PeerViewItem & pvi = (const PeerViewItem &) other;
		const PeerInterface::Stats & s = stats;
		const PeerInterface::Stats & os = pvi.stats;
		switch (treeWidget()->sortColumn())
		{
		case 0: 
		case 1:
		case 2: 
			return QTreeWidgetItem::operator < (other);
		case 3: return s.download_rate < os.download_rate;
		case 4: return s.upload_rate < os.upload_rate;
		case 5: return s.choked < os.choked;
		case 6: return s.snubbed < os.snubbed;
		case 7: return s.perc_of_file < os.perc_of_file;
		case 8: return s.dht_support < os.dht_support;
		case 9: return s.aca_score < os.aca_score;
		case 10: return s.has_upload_slot < os.has_upload_slot;
		case 11: return s.num_down_requests+s.num_up_requests < os.num_down_requests+os.num_up_requests;
		case 12: return s.bytes_downloaded < os.bytes_downloaded; 
		case 13: return s.bytes_uploaded < os.bytes_uploaded;
		default:
			 return false;
		}
	}


	void PeerViewItem::update(bool init)
	{
		const PeerInterface::Stats & s = peer->getStats();
		KLocale* loc = KGlobal::locale();

		if (init || s.download_rate != stats.download_rate)
		{
			if (s.download_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
				setText(3,KBytesPerSecToString(s.download_rate / 1024.0));
			else
				setText(3,"");
		}
		
		if (init || s.upload_rate != stats.upload_rate)
		{
			if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
				setText(4,KBytesPerSecToString(s.upload_rate / 1024.0));
			else
				setText(4,"");
		}

		if (init || s.choked != stats.choked)
			setText(5,s.choked ? i18nc("Choked", "Yes") : i18nc("Not choked", "No"));

		if (init || s.snubbed != stats.snubbed)
			setText(6,s.snubbed ? i18nc("Snubbed", "Yes") : i18nc("Not snubbed", "No"));

		if (init || s.perc_of_file != stats.perc_of_file)
			setText(7,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));

		if (init || s.aca_score != stats.aca_score)
			setText(9,loc->formatNumber(s.aca_score,2));

		if (init || s.has_upload_slot != stats.has_upload_slot)
			setIcon(10,s.has_upload_slot ? yes : KIcon());

		if (init || s.num_down_requests != stats.num_down_requests || s.num_up_requests != stats.num_up_requests)
			setText(11,QString("%1 / %2").arg(s.num_down_requests).arg(s.num_up_requests));

		if (init || s.bytes_downloaded != stats.bytes_downloaded)
			setText(12,BytesToString(s.bytes_downloaded));

		if (init || s.bytes_uploaded != stats.bytes_uploaded)
			setText(13,BytesToString(s.bytes_uploaded));

		stats = s;
	}

	PeerView::PeerView(QWidget* parent) : QTreeWidget(parent)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);

		QStringList columns;
		columns << i18n("IP Address")
			<< i18n("Country")
			<< i18n("Client")
			<< i18n("Down Speed")
			<< i18n("Up Speed")
			<< i18n("Choked")
			<< i18n("Snubbed")
			<< i18n("Availability")
			<< i18n("DHT")
			<< i18n("Score")
			<< i18n("Upload Slot")
			<< i18n("Requests")
			<< i18n("Downloaded")
			<< i18n("Uploaded");

		setHeaderLabels(columns);
		
		context_menu = new KMenu(this);
		context_menu->addAction(KIcon("list-remove-user"),i18n("Kick Peer"),this,SLOT(kickPeer()));
		context_menu->addAction(KIcon("view-filter"),i18n("Ban Peer"),this,SLOT(banPeer()));
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
	}

	PeerView::~PeerView()
	{
	}
	
	void PeerView::showContextMenu(const QPoint& pos)
	{
		if (!currentItem())
			return;
		
		context_menu->popup(mapToGlobal(pos));
	}
	
	void PeerView::banPeer()
	{
		QTreeWidgetItem* cur = currentItem();
		if (cur)
		{
			PeerViewItem* pv = (PeerViewItem*)cur;
			pv->peer->kill();
		}
	}
	
	void PeerView::kickPeer()
	{
		QTreeWidgetItem* cur = currentItem();
		if (cur)
		{
			PeerViewItem* pv = (PeerViewItem*)cur;
			pv->peer->kill();
			IPBlocklist& filter = IPBlocklist::instance();
			filter.insert(pv->stats.ip_address,3);
		}
	}

	void PeerView::peerAdded(PeerInterface* peer)
	{
		items.insert(peer, new PeerViewItem(this,peer));
	}

	void PeerView::peerRemoved(PeerInterface* peer)
	{
		PeerViewItem* v = items.find(peer);
		if (v)
		{
			items.erase(peer);
			delete v;
		}
	}

	void PeerView::update()
	{
		 bt::PtrMap<PeerInterface*,PeerViewItem>::iterator i = items.begin();
		 while (i != items.end())
		 {
			 i->second->update(false);
			 i++;
		 }
	}

	void PeerView::removeAll()
	{
		items.clear();
		clear();
	}

	void PeerView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("PeerView");
		QByteArray s = header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void PeerView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("PeerView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			header()->restoreState(s);
	}
}

#include "peerview.moc"
