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
#include <kicon.h>
#include <kmimetype.h>
#include <util/log.h>
#include <util/constants.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include "mediamodel.h"

using namespace bt;

namespace kt
{

	MediaModel::MediaModel(CoreInterface* core,QObject* parent) : QAbstractItemModel(parent),core(core)
	{
		QueueManager* qman = core->getQueueManager();
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* tc = *i;
			Item* item = new Item(*i);
			if (tc->getStats().multi_file_torrent && item->multimedia_files.count() > 0)
				items.append(item);
			else if (!tc->getStats().multi_file_torrent && tc->isMultimedia())
				items.append(item);
			else
				delete item;
		}
	}


	MediaModel::~MediaModel()
	{
		qDeleteAll(items);
	}
	
	MediaModel::Item::Item(bt::TorrentInterface* tc) : tc(tc)
	{
		if (tc->getStats().multi_file_torrent)
		{
			for (Uint32 i = 0;i < tc->getNumFiles();i++)
			{
				if (tc->getTorrentFile(i).isMultimedia())
					multimedia_files.append(i);
			}
		}
	}

	int MediaModel::rowCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return items.count();
		else if (!parent.internalPointer() && parent.row() >= 0 && parent.row() < items.count())
			return items.at(parent.row())->multimedia_files.count();
		else
			return 0;
	}
	
	int MediaModel::columnCount(const QModelIndex & parent) const
	{
		Q_UNUSED(parent);
		return 1;
	}
	
	QVariant MediaModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		Q_UNUSED(section);
		Q_UNUSED(orientation);
		Q_UNUSED(role);
		return QVariant();
	}
	
	QVariant MediaModel::data(const QModelIndex & index, int role) const
	{
		if (index.column() != 0)
			return QVariant();
		
		Item* item = (Item*)index.internalPointer();
		if (!item)
		{
			if (index.row() >= 0 && index.row() < items.count())
				item = items.at(index.row());
			else
				return QVariant();
			
			const bt::TorrentStats & s = item->tc->getStats();
			switch (role)
			{
				case Qt::DisplayRole:
					return s.torrent_name;
				case Qt::DecorationRole:
					return (item->multimedia_files.count() > 0) ? KIcon("folder") :  KIcon(KMimeType::findByPath(s.torrent_name)->iconName());
				default:
					return QVariant();
			}
		}
		else if (index.row() >= 0 && index.row() < item->multimedia_files.count())
		{
			int idx = item->multimedia_files.at(index.row());
			if (idx < 0 || idx >= item->tc->getNumFiles())
				return QVariant();
			
			QString path = item->tc->getTorrentFile(idx).getPath();
			switch (role)
			{
				case Qt::DisplayRole:
					return path;
				case Qt::DecorationRole:
					return KIcon(KMimeType::findByPath(path)->iconName());
				default:
					return QVariant();
			}
		}
		
		return QVariant();
	}
	
	bool MediaModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
		
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
		{
			if (row >= 0 && row < items.count())
			{
				Item* item = items[row];
				items.removeAt(row);
				delete item;
			}
		}
		endRemoveRows();
		return true;
	}
	
	bool MediaModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
					
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	QModelIndex MediaModel::index(int row,int column,const QModelIndex & parent) const
	{
		if (column != 0)
			return QModelIndex();
		
		if (!parent.isValid() && row >= 0 && row < items.count())
		{
			return createIndex(row,column); // it's a torrent
		}
		else if (parent.isValid() && parent.row() >= 0 && parent.row() < items.count() && !parent.internalPointer())
		{
			Item* item = items.at(parent.row());
			if (row >= 0 && row < item->multimedia_files.count())
				return createIndex(row,column,item);
			else
				return QModelIndex();
		}
		
		return QModelIndex();
	}
	
	QModelIndex MediaModel::parent(const QModelIndex & index) const
	{
		Item* item = (Item*)index.internalPointer();
		if (!item)
			return QModelIndex();
		else
			return createIndex(items.indexOf(item),0);
	}
	
	void MediaModel::onTorrentAdded(bt::TorrentInterface* tc)
	{
		Item* item = new Item(tc);
		if (tc->getStats().multi_file_torrent && item->multimedia_files.count() > 0)
		{
			items.append(item);
			insertRow(items.count());
		}
		else if (!tc->getStats().multi_file_torrent && tc->isMultimedia())
		{
			items.append(item);
			insertRow(items.count());
		}
		else
		{
			delete item;
		}
	}
	
	void MediaModel::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		Uint32 idx = 0;
		foreach (Item* i,items)
		{
			if (i->tc == tc)
			{
				removeRow(idx);
				return;
			}
			idx++;
		}
	}
	
	QString MediaModel::pathForIndex(const QModelIndex & idx) const
	{
		Item* item = (Item*)idx.internalPointer();
		if (item)
		{
			int r = idx.row();
			if (r < 0 || r >= item->tc->getNumFiles())
				return QString();
			else
				return item->tc->getTorrentFile(r).getPathOnDisk();
		}
		else
		{
			int r = idx.row();
			if (r < 0 || r >= items.count())
				return QString();
			
			bt::TorrentInterface* tc = items.at(r)->tc;
			if (!tc->getStats().multi_file_torrent)
				return tc->getStats().output_path;
			else
				return QString(); // we can't play directories
		}
	}
	
	QModelIndex MediaModel::indexForPath(const QString & path) const
	{
		Uint32 idx = 0;
		foreach (Item* i,items)
		{
			bt::TorrentInterface* tc = i->tc;
			if (!tc->getStats().multi_file_torrent)
			{
				if (path == tc->getStats().output_path)
					return index(idx,0,QModelIndex());
			}
			else
			{
				foreach (int j,i->multimedia_files)
				{
					if (tc->getTorrentFile(j).getPathOnDisk() == path)
						return index(j,0,index(idx,0,QModelIndex()));
				}
			}
			idx++;
		}
		
		return QModelIndex();
	}
	
	QModelIndex MediaModel::next(const QModelIndex & idx) const
	{
		if (!idx.isValid())
		{
			if (items.count() == 0)
				return QModelIndex();
			
			Item* f = items.at(0);
			if (!f->tc->getStats().multi_file_torrent)
				return index(0,0,QModelIndex());
			else
				return index(0,0,index(0,0,QModelIndex()));
		}
		else
		{
			QModelIndex	n = idx.sibling(idx.row()+1,0); // take a look at the next sibling
			if (!n.isValid())
			{
				n = parent(idx);
				n = n.sibling(n.row()+1,0);
				if (n.isValid() && n.child(0,0).isValid())
					n = n.child(0,0);
			}
			
			return n;
		}
	}
			
}
