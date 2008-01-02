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
#include <klocale.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include <util/functions.h>
#include "chunkdownloadmodel.h"

using namespace bt;

namespace kt
{
	
	ChunkDownloadModel::Item::Item(ChunkDownloadInterface* cd,const QString & files) : cd(cd),files(files)
	{
		cd->getStats(stats);
	}
			
	bool ChunkDownloadModel::Item::changed() const
	{
		ChunkDownloadInterface::Stats s;
		cd->getStats(s);

		if (s.pieces_downloaded != stats.pieces_downloaded || 
			s.download_speed != stats.download_speed || 
			s.num_downloaders != stats.num_downloaders || 
			s.current_peer_id != stats.current_peer_id)
		{
			stats = s;
			return true;
		}
		return false;
	}
	
	QVariant ChunkDownloadModel::Item::data(int col) const
	{
		switch (col)
		{
			case 0: return stats.chunk_index;
			case 1: return QString("%1 / %2").arg(stats.pieces_downloaded).arg(stats.total_pieces);
			case 2: return stats.current_peer_id;
			case 3: return KBytesPerSecToString(stats.download_speed / 1024.0);
			case 4: return stats.num_downloaders;
			case 5: return files;
		}
		return QVariant();
	}
	
	QVariant ChunkDownloadModel::Item::dataForSorting(int col) const
	{
		switch (col)
		{
			case 0: return stats.chunk_index;
			case 1: return stats.pieces_downloaded;
			case 2: return stats.current_peer_id;
			case 3: return stats.download_speed;
			case 4: return stats.num_downloaders;
			case 5: return files;
		}
		return QVariant();
	}
	
	/////////////////////////////////////////////////////////////

	ChunkDownloadModel::ChunkDownloadModel ( QObject* parent )
			: QAbstractTableModel(parent),tc(0)
	{
	}


	ChunkDownloadModel::~ChunkDownloadModel()
	{
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
						files += "\n";
						
					files += tf.getPath();
					n++;
				}
			}
		}
		
		items.append(Item(cd,files));
		insertRow(items.count() - 1);
	}
	
	void ChunkDownloadModel::downloadRemoved(bt::ChunkDownloadInterface* cd)
	{
		int idx = 0;
		for (QList<Item>::iterator i = items.begin();i != items.end();i++)
		{
			const Item & item = *i;
			if (item.cd == cd)
			{
				items.erase(i);
				removeRow(idx);
				break;
			}
			idx++;
		}
	}
	
	void ChunkDownloadModel::changeTC(bt::TorrentInterface* tc)
	{
		items.clear();
		this->tc = tc;
		reset();
	}
	
	void ChunkDownloadModel::clear()
	{
		items.clear();
		reset();
	}
	
	void ChunkDownloadModel::update()
	{
		Uint32 idx=0;
		foreach (const Item & i,items)
		{
			if (i.changed())
				emit dataChanged(createIndex(idx,1),createIndex(idx,4));
			idx++;
		}
	}

	int ChunkDownloadModel::rowCount ( const QModelIndex & parent ) const
	{
		if (parent.isValid())
			return 0;
		else
			return items.count();
	}
	
	int ChunkDownloadModel::columnCount ( const QModelIndex & parent ) const
	{
		if (parent.isValid())
			return 0;
		else
			return 6;
	}
	
	QVariant ChunkDownloadModel::headerData ( int section, Qt::Orientation orientation,int role ) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Chunk");
			case 1: return i18n("Progress");
			case 2: return i18n("Peer");
			case 3: return i18n("Down Speed");
			case 4: return i18n("Assigned Peers");
			case 5: return i18n("Files");
			default: return QVariant();
		}
	}
	
	QVariant ChunkDownloadModel::data ( const QModelIndex & index,int role ) const
	{
		if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::DisplayRole)
			return items[index.row()].data(index.column());
		else if (role == Qt::UserRole) // UserRole is for sorting
			return items[index.row()].dataForSorting(index.column());
		
		return QVariant();
	}
	
	bool ChunkDownloadModel::removeRows ( int row,int count,const QModelIndex & /*parent*/ )
	{
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool ChunkDownloadModel::insertRows ( int row,int count,const QModelIndex & /*parent*/ )
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}

}
