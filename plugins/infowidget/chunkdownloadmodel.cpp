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
#include "chunkdownloadmodel.h"

#include <klocale.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
	
	ChunkDownloadModel::Item::Item(ChunkDownloadInterface* cd,const QString & files) : cd(cd),files(files)
	{
		cd->getStats(stats);
	}
			
	bool ChunkDownloadModel::Item::changed(int col,bool & modified) const
	{
		ChunkDownloadInterface::Stats s;
		cd->getStats(s);
		bool ret = false;
		switch (col)
		{
			case 1: ret = s.pieces_downloaded != stats.pieces_downloaded; break;
			case 2: ret = s.current_peer_id != stats.current_peer_id; break;
			case 3: ret = s.download_speed != stats.download_speed; break;
			default: break;
		}

		modified = s.pieces_downloaded != stats.pieces_downloaded || 
			s.download_speed != stats.download_speed || 
			s.current_peer_id != stats.current_peer_id;
		
		stats = s;
		return ret;
	}
	
	QVariant ChunkDownloadModel::Item::data(int col) const
	{
		switch (col)
		{
			case 0: return stats.chunk_index;
			case 1: return QString("%1 / %2").arg(stats.pieces_downloaded).arg(stats.total_pieces);
			case 2: return stats.current_peer_id;
			case 3: return BytesPerSecToString(stats.download_speed);
			case 4: return files;
		}
		return QVariant();
	}
	
	bool ChunkDownloadModel::Item::lessThan(int col,const Item* other) const
	{
		switch (col)
		{
			case 0: return stats.chunk_index < other->stats.chunk_index;
			case 1: return stats.pieces_downloaded < other->stats.pieces_downloaded;
			case 2: return stats.current_peer_id < other->stats.current_peer_id;
			case 3: return stats.download_speed < other->stats.download_speed;
			case 4: return files < other->files;
		}
		return false;
	}
	
	/////////////////////////////////////////////////////////////

	ChunkDownloadModel::ChunkDownloadModel ( QObject* parent )
			: QAbstractTableModel(parent),tc(0)
	{
		sort_column = 0;
		sort_order = Qt::AscendingOrder;
	}


	ChunkDownloadModel::~ChunkDownloadModel()
	{
		qDeleteAll(items);
	}
	
	void ChunkDownloadModel::downloadAdded(bt::ChunkDownloadInterface* cd)
	{
		if (!tc)
			return;
		
		bt::ChunkDownloadInterface::Stats stats;
		cd->getStats(stats);
		QString files;
		int n = 0;
		if (tc->getStats().multi_file_torrent)
		{
			for (Uint32 i = 0;i < tc->getNumFiles();i++)
			{
				const bt::TorrentFileInterface & tf = tc->getTorrentFile(i);
				if (stats.chunk_index >= tf.getFirstChunk() && stats.chunk_index <= tf.getLastChunk())
				{
					if (n > 0)
						files += '\n';
						
					files += tf.getPath();
					n++;
				}
				else if (stats.chunk_index < tf.getFirstChunk())
					break;
			}
		}
		
		Item* nitem = new Item(cd,files);
		items.append(nitem);
		insertRow(items.count() - 1);	
		sort(sort_column,sort_order);
	}
	
	void ChunkDownloadModel::downloadRemoved(bt::ChunkDownloadInterface* cd)
	{
		int idx = 0;
		for (QList<Item*>::iterator i = items.begin();i != items.end();i++)
		{
			const Item* item = *i;
			if (item->cd == cd)
			{
				items.erase(i);
				delete item;
				removeRow(idx);
				break;
			}
			idx++;
		}
	}
	
	void ChunkDownloadModel::changeTC(bt::TorrentInterface* tc)
	{
		qDeleteAll(items);
		items.clear();
		this->tc = tc;
		reset();
	}
	
	void ChunkDownloadModel::clear()
	{
		qDeleteAll(items);
		items.clear();
		reset();
	}
	
	void ChunkDownloadModel::update()
	{
		bool resort = false;
		Uint32 idx=0;
		foreach (Item* i,items)
		{
			bool modified = false;
			if (i->changed(sort_column,modified))
				resort = true;
			
			if (modified && !resort)
				emit dataChanged(index(idx,1),index(idx,3));
			idx++;
		}
	
		if (resort)
			sort(sort_column,sort_order);
	}

	int ChunkDownloadModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return items.count();
	}
	
	int ChunkDownloadModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 5;
	}
	
	QVariant ChunkDownloadModel::headerData(int section,Qt::Orientation orientation,int role) const
	{
		if (orientation != Qt::Horizontal)
			return QVariant();
		 
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
				case 0: return i18n("Chunk");
				case 1: return i18n("Progress");
				case 2: return i18n("Peer");
				case 3: return i18n("Down Speed");
				case 4: return i18n("Files");
				default: return QVariant();
			}
		}
		else if (role == Qt::ToolTipRole)
		{
			switch (section)
			{
				case 0: return i18n("Number of the chunk");
				case 1: return i18n("Download progress of the chunk");
				case 2: return i18n("Which peer we are downloading it from");
				case 3: return i18n("Download speed of the chunk");
				case 4: return i18n("Which files the chunk is located in");
				default: return QVariant();
			}
		}
		
		return QVariant();
	}
	
	QModelIndex ChunkDownloadModel::index(int row,int column,const QModelIndex & parent) const
	{
		if (!hasIndex(row,column,parent) || parent.isValid())
			return QModelIndex();
		else
			return createIndex(row,column,items[row]);
	}
	
	QVariant ChunkDownloadModel::data(const QModelIndex & index,int role) const
	{
		if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::DisplayRole)
			return items[index.row()]->data(index.column());
				
		return QVariant();
	}
	
	bool ChunkDownloadModel::removeRows(int row,int count,const QModelIndex & /*parent*/ )
	{
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool ChunkDownloadModel::insertRows(int row,int count,const QModelIndex & /*parent*/ )
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	class ChunkDownloadModelItemCmp
	{
	public:
		ChunkDownloadModelItemCmp(int col,Qt::SortOrder order) : col(col),order(order)
		{}
	
		bool operator()(ChunkDownloadModel::Item* a,ChunkDownloadModel::Item* b)
		{
			if (order == Qt::AscendingOrder)
				return a->lessThan(col,b);
			else
				return !a->lessThan(col,b);
		}
	
		int col;
		Qt::SortOrder order;
	};

	void ChunkDownloadModel::sort(int col, Qt::SortOrder order)
	{
		sort_column = col;
		sort_order = order;
		emit layoutAboutToBeChanged();
		qStableSort(items.begin(),items.end(),ChunkDownloadModelItemCmp(col,order));
		emit layoutChanged();
	}
}
