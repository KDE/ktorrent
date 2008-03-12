/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <klocale.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>
#include "peerviewmodel.h"
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
	

	PeerViewModel::Item::Item(bt::PeerInterface* peer) : peer(peer)
	{
		stats = peer->getStats();
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
			int country_id = GeoIP_id_by_name(geo_ip, stats.ip_address.toAscii());
			country = GeoIP_country_name[country_id];
			flag = KIcon(flagDB.getFlag(GeoIP_country_code[country_id]));
		}
	}
			
	bool PeerViewModel::Item::changed() const
	{
		const PeerInterface::Stats & s = peer->getStats();
		

		if (s.download_rate != stats.download_rate || 
			s.upload_rate != stats.upload_rate || 
			s.choked != stats.choked || 
			s.snubbed != stats.snubbed || 
			s.perc_of_file != stats.perc_of_file || 
			s.aca_score != stats.aca_score || 
			s.has_upload_slot != stats.has_upload_slot || 
			s.num_down_requests != stats.num_down_requests || 
			s.num_up_requests != stats.num_up_requests || 
			s.bytes_downloaded != stats.bytes_downloaded || 
			s.bytes_uploaded != stats.bytes_uploaded)
		{
			stats = s;
			return true;
		}
		return false;
	}
	
	QVariant PeerViewModel::Item::data(int col) const
	{
		switch (col)
		{
			case 0: return stats.ip_address;
			case 1: return country;
			case 2: return stats.client;
			case 3: return KBytesPerSecToString(stats.download_rate / 1024.0);
			case 4: return KBytesPerSecToString(stats.upload_rate / 1024.0);
			case 5: return stats.choked ? i18nc("Choked", "Yes") : i18nc("Not choked", "No");
			case 6: return stats.snubbed ? i18nc("Snubbed", "Yes") : i18nc("Not snubbed", "No");
			case 7: return QString("%1 %").arg(KGlobal::locale()->formatNumber(stats.perc_of_file,2));
			case 8: return QVariant();
			case 9: return KGlobal::locale()->formatNumber(stats.aca_score,2);
			case 10: return QVariant(); 
			case 11: return QString("%1 / %2").arg(stats.num_down_requests).arg(stats.num_up_requests);
			case 12: return BytesToString(stats.bytes_downloaded);
			case 13: return BytesToString(stats.bytes_uploaded);
			default: return QVariant();
		}
		return QVariant();
	}
	
	QVariant PeerViewModel::Item::dataForSorting(int col) const
	{
		switch (col)
		{
			case 0: return stats.ip_address;
			case 1: return country;
			case 2: return stats.client;
			case 3: return stats.download_rate;
			case 4: return stats.upload_rate;
			case 5: return stats.choked;
			case 6: return stats.snubbed;
			case 7: return stats.perc_of_file;
			case 8: return stats.dht_support;
			case 9: return stats.aca_score;
			case 10: return stats.has_upload_slot;
			case 11: return stats.num_down_requests + stats.num_up_requests;
			case 12: return stats.bytes_downloaded;
			case 13: return stats.bytes_uploaded;
			default: return QVariant();
		}
		return QVariant();
	}
	
	QVariant PeerViewModel::Item::decoration(int col) const
	{
		switch (col)
		{
			case 0:
				if (stats.encrypted)
					return KIcon("ktencrypted");
				break;
			case 1: return flag;
			case 8: return stats.dht_support ? yes : no;
			case 10: return stats.has_upload_slot ? yes : KIcon();
		}
		
		return QVariant();
	}
	
	/////////////////////////////////////////////////////////////

	PeerViewModel::PeerViewModel ( QObject* parent )
	: QAbstractTableModel(parent)
	{
	}


	PeerViewModel::~PeerViewModel()
	{
	}
	
	void PeerViewModel::peerAdded(bt::PeerInterface* peer)
	{
		items.append(Item(peer));
		insertRow(items.count() - 1);
	}
	
	void PeerViewModel::peerRemoved(bt::PeerInterface* peer)
	{
		int idx = 0;
		for (QList<Item>::iterator i = items.begin();i != items.end();i++)
		{
			const Item & item = *i;
			if (item.peer == peer)
			{
				items.erase(i);
				removeRow(idx);
				break;
			}
			idx++;
		}
	}
	
	void PeerViewModel::clear()
	{
		items.clear();
		reset();
	}
	
	bool PeerViewModel::update()
	{
		bool ret = false;
		Uint32 idx=0;
		foreach (const Item & i,items)
		{
			if (i.changed())
			{
				ret = true;
				emit dataChanged(createIndex(idx,3),createIndex(idx,13));
			}
			idx++;
		}
		return ret;
	}

	int PeerViewModel::rowCount( const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return items.count();
	}
	
	int PeerViewModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 14;
	}
	
	QVariant PeerViewModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("IP Address");
			case 1: return i18n("Country");
			case 2: return i18n("Client");
			case 3: return i18n("Down Speed");
			case 4: return i18n("Up Speed");
			case 5: return i18n("Choked");
			case 6: return i18n("Snubbed");
			case 7: return i18n("Availability");
			case 8: return i18n("DHT");
			case 9: return i18n("Score");
			case 10: return i18n("Upload Slot");
			case 11: return i18n("Requests");
			case 12: return i18n("Downloaded");
			case 13: return i18n("Uploaded");
			default: return QVariant();
		}
	}
	
	QVariant PeerViewModel::data(const QModelIndex & index,int role) const
	{
		if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::DisplayRole)
			return items[index.row()].data(index.column());
		else if (role == Qt::UserRole) // UserRole is for sorting
			return items[index.row()].dataForSorting(index.column());
		else if (role == Qt::DecorationRole)
			return items[index.row()].decoration(index.column());
		
		return QVariant();
	}
	
	bool PeerViewModel::removeRows ( int row,int count,const QModelIndex & /*parent*/ )
	{
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool PeerViewModel::insertRows ( int row,int count,const QModelIndex & /*parent*/ )
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}

	bt::PeerInterface* PeerViewModel::indexToPeer(const QModelIndex & index)
	{
		if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
			return 0; 
		else
			return items[index.row()].peer;
	}

}
