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
#include <groups/group.h>
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
		hidden = false;
	}
			
	
	bool ViewModel::Item::update(int col,bool & modified)
	{
		bool ret = false;
		const TorrentStats & s = tc->getStats();
		if (status != s.status)
		{
			modified = true;
			status = s.status;
			if (col == STATUS)
				ret = true;
		}
			
		if (bytes_downloaded != s.bytes_downloaded)
		{
			modified = true;
			bytes_downloaded = s.bytes_downloaded;
			if (col == BYTES_DOWNLOADED)
				ret = true;
		}
		
		if (total_bytes_to_download != s.total_bytes_to_download)
		{
			modified = true;
			total_bytes_to_download = s.total_bytes_to_download;
			if (col == TOTAL_BYTES_TO_DOWNLOAD)
				ret = true;
		}
		
		if (bytes_uploaded != s.bytes_uploaded)
		{
			modified = true;
			bytes_uploaded = s.bytes_uploaded;
			if (col == BYTES_UPLOADED)
				ret = true;
		}
		
		if (download_rate != s.download_rate)
		{
			modified = true;
			download_rate = s.download_rate;
			if (col == DOWNLOAD_RATE)
				ret = true;
		}
		
		if (upload_rate != s.upload_rate)
		{
			modified = true;
			upload_rate = s.upload_rate;
			if (col == UPLOAD_RATE)
				ret = true;
		}
		
		int neta = tc->getETA();
		if (eta != neta)
		{
			modified = true;
			eta = neta;
			if (col == ETA)
				ret = true;
		}
		
		if (seeders_connected_to != s.seeders_connected_to || seeders_total != s.seeders_total)
		{
			modified = true;
			seeders_connected_to = s.seeders_connected_to;
			seeders_total = s.seeders_total;
			if (col == SEEDERS)
				ret = true;
		}
		
		if (leechers_total != s.leechers_total || leechers_connected_to != s.leechers_connected_to)
		{
			modified = true;
			leechers_total = s.leechers_total;
			leechers_connected_to = s.leechers_connected_to;
			if (col == LEECHERS)
				ret = true;
		}
		
		double perc = Percentage(s); 
		if (fabs(percentage - perc) > 0.01)
		{
			modified = true;
			percentage = perc;
			if (col == PERCENTAGE)
				ret = true;
		}
		
		float ratio = ShareRatio(s);
		if (fabsf(share_ratio - ratio) > 0.01)
		{
			modified = true;
			share_ratio = ratio;
			if (col == SHARE_RATIO)
				ret = true;
		}
		
		Uint32 rdl = tc->getRunningTimeDL();
		if (runtime_dl != rdl)
		{
			modified = true;
			runtime_dl = rdl;
			if (col == DOWNLOAD_TIME)
				ret = true;
		}
		
		Uint32 rul = tc->getRunningTimeUL();
		rul = rul >= rdl ? rul - rdl : 0; // make sure rul cannot go negative
		if (runtime_ul != rul)
		{
			modified = true;
			runtime_ul = rul;
			if (col == SEED_TIME)
				ret = true;
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
					return BytesPerSecToString(download_rate);
				else
					return QVariant();
				break;
			case 6: 
				if (upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
					return BytesPerSecToString(upload_rate);
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
	
	bool ViewModel::Item::lessThan(int col,const Item* other) const
	{
		switch (col)
		{
			case 0: return QString::localeAwareCompare(tc->getDisplayName(),other->tc->getDisplayName()) < 0;
			case 1: return tc->statusToString() < other->tc->statusToString();
			case 2: return bytes_downloaded < other->bytes_downloaded;
			case 3: return total_bytes_to_download < other->total_bytes_to_download;
			case 4: return bytes_uploaded < other->bytes_uploaded;
			case 5: return (download_rate < 102 ? 0 : download_rate) < (other->download_rate < 102 ? 0 : other->download_rate);
			case 6: return (upload_rate < 102 ? 0 : upload_rate) < (other->upload_rate < 102 ? 0 : other->upload_rate);
			case 7: return eta < other->eta;	
			case 8: return seeders_connected_to < other->seeders_connected_to;
			case 9: return leechers_connected_to < other->leechers_connected_to;
			case 10: return percentage < other->percentage;
			case 11: return share_ratio < other->share_ratio;
			case 12: return runtime_dl < other->runtime_dl;
			case 13: return runtime_ul < other->runtime_ul;
			case 14: return tc->getStats().output_path < other->tc->getStats().output_path;
			default: return false;
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
	
	bool ViewModel::Item::member(Group* group) const
	{
		if (!group)
			return true;
		else
			return group->isMember(tc);
	}

	ViewModel::ViewModel(Core* core,QObject* parent) : QAbstractTableModel(parent),core(core)
	{
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(addTorrent(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(removeTorrent(bt::TorrentInterface*)));
		sort_column = 0;
		sort_order = Qt::AscendingOrder;
		group = 0;
		num_visible = 0;
		
		kt::QueueManager* qman = core->getQueueManager();
		for (QList<bt::TorrentInterface*>::iterator i = qman->begin();i != qman->end();i++)
		{
			torrents.append(new Item(*i));
			num_visible++;
		}
	}


	ViewModel::~ViewModel()
	{
		qDeleteAll(torrents);
	}
	
	void ViewModel::setGroup(Group* g)
	{
		group = g;
		update();
	}
	
	void ViewModel::addTorrent(bt::TorrentInterface* ti)
	{
		torrents.append(new Item(ti));
		update(true);
	}
	
	void ViewModel::removeTorrent(bt::TorrentInterface* ti)
	{
		int idx = 0;
		for (QList<Item*>::iterator i = torrents.begin();i != torrents.end();i++)
		{
			Item* item = *i;
			if (item->tc == ti)
			{
				removeRow(idx);
				torrents.erase(i);
				delete item;
				update(true);
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

	void ViewModel::update(bool force_resort)
	{
		bool resort = force_resort;
		Uint32 idx=0;
		num_visible = 0;
		foreach (Item* i,torrents)
		{
			bool modified = false;
			if (i->update(sort_column,modified))
				resort = true;
			
			bool hidden = !i->member(group);
			if (hidden != i->hidden)
			{
				i->hidden = hidden;
				resort = true;
			}
			
			if (!i->hidden)
				num_visible++;
			
			if (modified && !resort)
				emit dataChanged(index(idx,1),index(idx,14));
			idx++;
		}
	
		if (resort)
		{
			sort(sort_column,sort_order);
		}
	}
	
	int ViewModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return num_visible;
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
		if (orientation != Qt::Horizontal)
			return QVariant();
		 
		if (role == Qt::DisplayRole)
		{
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
		else if (role == Qt::ToolTipRole)
		{
			switch (section)
			{
				case 2: return i18n("How much data we have downloaded of the torrent");
				case 3: return i18n("Total size of the torrent, excluded files are not included");
				case 4: return i18n("How much data we have uploaded");
				case 5: return i18n("Current download speed");
				case 6: return i18n("Current upload speed");
				case 7: return i18n("How much time is left before the torrent is finished or before the maximum share ratio is reached, if that is enabled");
				case 8: return i18n("How many seeders we are connected to (How many seeders there are according to the tracker)");
				case 9: return i18n("How many leechers we are connected to (How many leechers there are according to the tracker)");
				// xgettext: no-c-format
				case 10: return i18n("The percentage of data we have of the whole torrent, not including excluded files");
				case 11: return i18n("Share ratio is the number of bytes uploaded divided by the number of bytes downloaded");
				case 12: return i18n("How long we have been downloading the torrent");
				case 13: return i18n("How long we have been seeding the torrent");
				case 14: return i18n("The location of the torrent's data on disk");
				default: return QVariant();
			}
		}
		
		return QVariant();
	}
	
	QModelIndex ViewModel::index(int row,int column,const QModelIndex & parent) const
	{
		if (parent.isValid())
			return QModelIndex();
		
		if (row < 0 || row >= torrents.count())
			return QModelIndex();
		else
			return createIndex(row,column,torrents[row]);
	}
	
	QVariant ViewModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0)
			return QVariant(); 
		
		Item* item = (Item*)index.internalPointer();
		if (!item)
			return QVariant(); 
		
		if (role == Qt::ForegroundRole)
			return item->color(index.column());
		else if (role == Qt::DisplayRole)
			return item->data(index.column());
		else if (role == Qt::EditRole && index.column() == 0)
			return item->tc->getDisplayName();
		else if (role == Qt::DecorationRole && index.column() == 1)
		{
			bt::TorrentInterface* tc = item->tc;
			if (tc->getStats().tracker_status == bt::TRACKER_ERROR)
				return KIcon("dialog-warning");
		} 
		else if (role == Qt::ToolTipRole)
		{
			if (index.column() == 1)
			{
				bt::TorrentInterface* tc = item->tc;
				if (tc->getStats().tracker_status == bt::TRACKER_ERROR)
					return i18n("There is a problem with the tracker: <br><b>%1</b>",tc->getStats().tracker_status_string);
			}
			else if (index.column() == 0)
			{
				bt::TorrentInterface* tc = item->tc;
				if (tc->loadUrl().isValid())
					return i18n("%1<br>Url: <b>%2</b>",tc->getDisplayName(),tc->loadUrl().prettyUrl());
				else
					return tc->getDisplayName();
			}
		}
		
		return QVariant();
	}
	
	bool ViewModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0 || 
			role != Qt::EditRole || index.column() != 0)
			return false; 
		
		QString name = value.toString();
		Item* item = (Item*)index.internalPointer();
		if (!item)
			return false;
		
		bt::TorrentInterface* tc = item->tc;
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
				Item* item = (Item*)i.internalPointer();
				if (item)
					tlist.append(item->tc);
			}
		}
	}
	
	const bt::TorrentInterface* ViewModel::torrentFromIndex(const QModelIndex & index) const
	{
		if (index.isValid() && index.row() < torrents.count() && index.row() >= 0 && index.internalPointer())
			return ((Item*)index.internalPointer())->tc;
		else
			return 0;
	}
	
	bt::TorrentInterface* ViewModel::torrentFromIndex(const QModelIndex & index)
	{
		if (index.isValid() && index.row() < torrents.count() && index.row() >= 0 && index.internalPointer())
			return ((Item*)index.internalPointer())->tc;
		else
			return 0;
	}
	
	bt::TorrentInterface* ViewModel::torrentFromRow(int index)
	{
		if (index < torrents.count() && index >= 0)
			return torrents[index]->tc;
		else
			return 0;
	}
	
	void ViewModel::allTorrents(QList<bt::TorrentInterface*> & tlist) const
	{
		for (QList<Item*>::const_iterator i = torrents.begin();i != torrents.end();i++)
		{
			Item* item = *i;
			if (item->member(group))
				tlist.append(item->tc);
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
	
	class ViewModelItemCmp
	{
	public:
		ViewModelItemCmp(int col,Qt::SortOrder order) : col(col),order(order)
		{}
	
		bool operator()(ViewModel::Item* a,ViewModel::Item* b)
		{
			if (a->hidden)
				return false;
			else if (b->hidden)
				return true;
			else if (order == Qt::AscendingOrder)
				return a->lessThan(col,b);
			else
				return !a->lessThan(col,b);
		}
	
		int col;
		Qt::SortOrder order;
	};
	
	void ViewModel::sort(int col, Qt::SortOrder order)
	{
		sort_column = col;
		sort_order = order;
		emit layoutAboutToBeChanged();
		qStableSort(torrents.begin(),torrents.end(),ViewModelItemCmp(col,order));
	/*	Out(SYS_GEN|LOG_DEBUG) << "Sort results:" << endl;
		foreach (Item* i,torrents)
			Out(SYS_GEN|LOG_DEBUG) << "Item: " << i->hidden << " " << i->tc->getDisplayName() << endl;
		*/
		emit layoutChanged();
	}
}

#include "viewmodel.moc"
