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
#include <math.h>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QMimeData>
#include <klocale.h>
#include <kglobal.h>
#include <kicon.h>
#include <util/log.h>
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include "viewmodel.h"
#include "core.h"

using namespace bt;

namespace kt
{

	
	ViewModel::Item::Item(bt::TorrentInterface* tc) : tc(tc)
	{
		const TorrentStats & s = tc->getStats();
		status = s.status;
		bytes_downloaded = s.bytes_downloaded;
		total_bytes_to_download = s.total_bytes_to_download;
		bytes_uploaded = s.bytes_uploaded;
		download_rate = s.download_rate;
		upload_rate = s.upload_rate;
		eta = tc->getETA();
		seeders_connected_to = s.seeders_connected_to;
		seeders_total = s.seeders_total;
		leechers_total = s.leechers_total;
		leechers_connected_to = s.leechers_connected_to;
		percentage = Percentage(s);
		share_ratio = ShareRatio(s);
		runtime_dl = tc->getRunningTimeDL();
		runtime_ul = tc->getRunningTimeUL() - tc->getRunningTimeDL();
	}
			
	
	bool ViewModel::Item::update()
	{
		bool ret = false;
		const TorrentStats & s = tc->getStats();
		if (status != s.status)
		{
			ret = true;
			status = s.status;
		}
			
		if (bytes_downloaded != s.bytes_downloaded)
		{
			ret = true;
			bytes_downloaded = s.bytes_downloaded;
		}
		
		if (total_bytes_to_download != s.total_bytes_to_download)
		{
			ret = true;
			total_bytes_to_download = s.total_bytes_to_download;
		}
		
		if (bytes_uploaded != s.bytes_uploaded)
		{
			ret = true;
			bytes_uploaded = s.bytes_uploaded;
		}
		
		if (download_rate != s.download_rate)
		{
			ret = true;
			download_rate = s.download_rate;
		}
		
		if (upload_rate != s.upload_rate)
		{
			ret = true;
			upload_rate = s.upload_rate;
		}
		
		int neta = tc->getETA();
		if (eta != neta)
		{
			ret = true;
			eta = neta;
		}
		
		if (seeders_connected_to != s.seeders_connected_to || seeders_total != s.seeders_total)
		{
			ret = true;
			seeders_connected_to = s.seeders_connected_to;
			seeders_total = s.seeders_total;
		}
		
		if (leechers_total != s.leechers_total || leechers_connected_to != s.leechers_connected_to)
		{
			ret = true;
			leechers_total = s.leechers_total;
			leechers_connected_to = s.leechers_connected_to;
		}
		
		double perc = Percentage(s); 
		if (fabs(percentage - perc) > 0.01)
		{
			ret = true;
			percentage = perc;
		}
		
		float ratio = ShareRatio(s);
		if (fabsf(share_ratio - ratio) > 0.01)
		{
			ret = true;
			share_ratio = ratio;
		}
		
		Uint32 rdl = tc->getRunningTimeDL();
		if (runtime_dl != rdl)
		{
			ret = true;
			runtime_dl = rdl;
		}
		
		Uint32 rul = tc->getRunningTimeUL();
		rul = rul >= rdl ? rul - rdl : 0; // make sure rul cannot go negative
		if (runtime_ul != rul)
		{
			ret = true;
			runtime_ul = rul;
		}
		return ret;
	}
	
	QVariant ViewModel::Item::data(int col) const
	{
		const TorrentStats & s = tc->getStats();
		switch (col)
		{
			case 0: return tc->getDisplayName();
			case 1: return tc->statusToString();
			case 2: return BytesToString(bytes_downloaded);
			case 3: return BytesToString(total_bytes_to_download);
			case 4: return BytesToString(bytes_uploaded);
			case 5: 
				if (download_rate >= 103 && s.bytes_left_to_download > 0) // lowest "visible" speed, all below will be 0,0 Kb/s
					return KBytesPerSecToString(download_rate / 1024.0);
				else
					return QVariant();
				break;
			case 6: 
				if (upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
					return KBytesPerSecToString(upload_rate / 1024.0);
				else
					return QVariant();
				break;
			case 7: 
				if (eta == 0)
					return QString("%1").arg(QChar(0x221E)); // infinity
				else if (eta > 0)
					return DurationToString(eta);
				else
					return QVariant();
				break;
			case 8: return QString("%1 (%2)").arg(QString::number(seeders_connected_to)).arg(QString::number(seeders_total));
			case 9: return QString("%1 (%2)").arg(QString::number(leechers_connected_to)).arg(QString::number(leechers_total));
			// xgettext: no-c-format
			case 10: return i18n("%1 %",KGlobal::locale()->formatNumber(percentage,2));
			case 11: return KGlobal::locale()->formatNumber(share_ratio,2);
			case 12: return DurationToString(runtime_dl);
			case 13: return DurationToString(runtime_ul);
			case 14: return tc->getStats().output_path;
			default: return QVariant();
		}
	}
	
	QVariant ViewModel::Item::dataForSorting(int col) const
	{
		const TorrentStats & s = tc->getStats();
		switch (col)
		{
			case 0: return tc->getDisplayName();
			case 1: return tc->statusToString();
			case 2: return bytes_downloaded;
			case 3: return total_bytes_to_download;
			case 4: return bytes_uploaded;
			case 5: 
				if (download_rate >= 103 && s.bytes_left_to_download > 0) // lowest "visible" speed, all below will be 0,0 Kb/s
					return download_rate;
				else
					return 0;
				break;
			case 6: 
				if (upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
					return upload_rate;
				else
					return 0;
				break;
			case 7: return eta;	
			case 8: return seeders_connected_to;
			case 9: return leechers_connected_to;
			// xgettext: no-c-format
			case 10: return percentage;
			case 11: return share_ratio;
			case 12: return runtime_dl;
			case 13: return runtime_ul;
			case 14: return tc->getStats().output_path;
			default: return QVariant();
		}
	}
	
	QVariant ViewModel::Item::color(int col) const
	{
		if (col == 1)
		{	
			QColor green(40,205,40);
			QColor yellow(255,174,0);
			
			switch (status)
			{
				case bt::SEEDING :
				case bt::DOWNLOADING:
				case bt::ALLOCATING_DISKSPACE :
					return green;
				case bt::STALLED:
				case bt::CHECKING_DATA:
					return yellow;
				case bt::ERROR :
					return Qt::red;
				case bt::NOT_STARTED :
				case bt::STOPPED:
				case bt::QUEUED:
				case bt::DOWNLOAD_COMPLETE :
				case bt::SEEDING_COMPLETE :
				default:
					return QVariant();
			}
		}
		else if (col == 11)
		{
			QColor green(40,205,40);
			return share_ratio > 0.8 ? green : Qt::red;
		}
		else
			return QVariant();
	}

	ViewModel::ViewModel(Core* core,QObject* parent) : QAbstractTableModel(parent),core(core),changed_values(false)
	{
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(addTorrent(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(removeTorrent(bt::TorrentInterface*)));
		
		kt::QueueManager* qman = core->getQueueManager();
		for (QList<bt::TorrentInterface*>::iterator i = qman->begin();i != qman->end();i++)
		{
			Item item(*i);
			torrents.append(item);
		}
	}


	ViewModel::~ViewModel()
	{}
	
	void ViewModel::addTorrent(bt::TorrentInterface* ti)
	{
		Item i(ti);
		torrents.append(i);
		insertRow(torrents.count() - 1);
	}
	
	void ViewModel::removeTorrent(bt::TorrentInterface* ti)
	{
		int idx = 0;
		for (QList<Item>::iterator i = torrents.begin();i != torrents.end();i++)
		{
			const Item & item = *i;
			if (item.tc == ti)
			{
				torrents.erase(i);
				removeRow(idx);
				break;
			}
			idx++;
		}
	}
	
	void ViewModel::emitDataChanged(int row,int col)
	{
		QModelIndex idx = createIndex(row,col);
		emit dataChanged(idx,idx);
		//emit dataChanged(createIndex(row,0),createIndex(row,14));
	}

	void ViewModel::update()
	{
		changed_values = false;
		int row = 0;
		for (QList<Item>::iterator i = torrents.begin();i != torrents.end();i++)
		{
			Item & item = *i;
			if (item.update())
				changed_values = true;
			row++;
		}
	}
	
	int ViewModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return torrents.count();
	}
	
	int ViewModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 15;
	}
	
	bool ViewModel::defaultColumnForUpload(int column)
	{
		return column != 2 && column != 5 && column != 10 && column != 12 && column != 14;
	}
	
	bool ViewModel::defaultColumnForDownload(int column)
	{
		return column != 13 && column != 14;
	}
	
	QVariant ViewModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Name");
			case 1: return i18n("Status");
			case 2: return i18n("Downloaded");
			case 3: return i18n("Size");
			case 4: return i18n("Uploaded");
			case 5: return i18n("Down Speed");
			case 6: return i18n("Up Speed");
			case 7: return i18n("Time Left");
			case 8: return i18n("Seeders");
			case 9: return i18n("Leechers");
			// xgettext: no-c-format
			case 10: return i18n("% Complete");
			case 11: return i18n("Share Ratio");
			case 12: return i18n("Time Downloaded");
			case 13: return i18n("Time Seeded");
			case 14: return i18n("Location");
			default: return QVariant();
		}
	}
	
	QVariant ViewModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::ForegroundRole)
			return torrents[index.row()].color(index.column());
		else if (role == Qt::DisplayRole)
			return torrents[index.row()].data(index.column());
		else if (role == Qt::UserRole) // UserRole is for sorting
			return torrents[index.row()].dataForSorting(index.column());
		else if (role == Qt::EditRole && index.column() == 0)
			return torrents[index.row()].tc->getDisplayName();
		else if (role == Qt::DecorationRole && index.column() == 1)
		{
			bt::TorrentInterface* tc = torrents[index.row()].tc;
			if (tc->getStats().tracker_status == bt::TRACKER_ERROR)
				return KIcon("dialog-warning");
		} 
		else if (role == Qt::ToolTipRole && index.column() == 1)
		{
			bt::TorrentInterface* tc = torrents[index.row()].tc;
			if (tc->getStats().tracker_status == bt::TRACKER_ERROR)
				return i18n("There is a problem with the tracker: <br><b>%1</b>",tc->getStats().tracker_status_string);
		}
		
		return QVariant();
	}
	
	bool ViewModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0 || 
			role != Qt::EditRole || index.column() != 0)
			return false; 
		
		QString name = value.toString();
		bt::TorrentInterface* tc = torrents[index.row()].tc;
		tc->setDisplayName(name);
		emit dataChanged(index,index);
		return true;
	}
	
	Qt::ItemFlags ViewModel::flags(const QModelIndex & index) const
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0)
			return QAbstractTableModel::flags(index);
		
		Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;
		if (index.column() == 0 )
			flags |= Qt::ItemIsEditable;
		
		return flags;
	}
	
	QStringList ViewModel::mimeTypes() const
	{
		QStringList types;
		types << "application/x-ktorrent-drag-object";
		return types;
	}
	
	QMimeData* ViewModel::mimeData(const QModelIndexList &indexes) const
	{
		QMimeData* mime_data = new QMimeData();
		QByteArray encoded_data;

		QDataStream stream(&encoded_data, QIODevice::WriteOnly);

		foreach (const QModelIndex &index, indexes) 
		{
			if (index.isValid()) 
			{
				QString text = data(createIndex(index.row(),0), Qt::DisplayRole).toString();
				stream << text;
			}
		}

		mime_data->setData( "application/x-ktorrent-drag-object", encoded_data);
		return mime_data;
	}
	
	void ViewModel::torrentsFromIndexList(const QModelIndexList & idx,QList<bt::TorrentInterface*> & tlist)
	{
		foreach (const QModelIndex &i,idx)
		{
			if (i.isValid())
			{
				tlist.append(torrents[i.row()].tc);
			}
		}
	}
	
	const bt::TorrentInterface* ViewModel::torrentFromIndex(const QModelIndex & index) const
	{
		if (index.isValid() && index.row() < torrents.count() && index.row() >= 0)
			return torrents[index.row()].tc;
		else
			return 0;
	}
	
	bt::TorrentInterface* ViewModel::torrentFromIndex(const QModelIndex & index)
	{
		if (index.isValid() && index.row() < torrents.count() && index.row() >= 0)
			return torrents[index.row()].tc;
		else
			return 0;
	}
	
	bt::TorrentInterface* ViewModel::torrentFromRow(int index)
	{
		if (index < torrents.count() && index >= 0)
			return torrents[index].tc;
		else
			return 0;
	}
	
	void ViewModel::allTorrents(QList<bt::TorrentInterface*> & tlist) const
	{
		for (QList<Item>::const_iterator i = torrents.begin();i != torrents.end();i++)
		{
			const Item & item = *i;
			tlist.append(item.tc);
		}
	}
	
	bool ViewModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool ViewModel::removeRows(int row,int count,const QModelIndex & parent) 
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
}

#include "viewmodel.moc"
