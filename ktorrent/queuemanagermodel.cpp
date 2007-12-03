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
#include <QColor>
#include <QMimeData>
#include <klocale.h>
#include <util/log.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "queuemanagermodel.h"

using namespace bt;

namespace kt
{

	QueueManagerModel::QueueManagerModel(QueueManager* qman,QObject* parent)
			: QAbstractTableModel(parent),qman(qman)
	{
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		torrents.sort();
		connect(qman,SIGNAL(queueOrdered()),this,SLOT(onQueueOrdered()));
	}


	QueueManagerModel::~QueueManagerModel()
	{}
	
	void QueueManagerModel::onQueueOrdered()
	{
		torrents.sort();
		reset();
	}
	
	void QueueManagerModel::onTorrentAdded(bt::TorrentInterface* tc)
	{
		torrents.append(tc);
		insertRow(torrents.count() - 1);
		reset();
	}
	
	void QueueManagerModel::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		int r = torrents.indexOf(tc);
		if (r >= 0)
			removeRow(r);
	}

	int QueueManagerModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return torrents.count();
	}
	
	int QueueManagerModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 2;
	}
	
	QVariant QueueManagerModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Name");
			case 1: return i18n("Status");
			case 2: return i18n("Priority");
			default: return QVariant();
		}
	}
	
	QVariant QueueManagerModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid() || index.row() >= torrents.count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::ForegroundRole)
		{
			bt::TorrentInterface* tc = torrents[index.row()];
			if (index.column() == 1)
			{
				if (tc->getPriority() == 0)
					return QVariant();
				else if (tc->getStats().running)
					return QColor(40,205,40); // green
				else
					return QColor(255,174,0); // yellow
			}
			return QVariant();
		}
		else if (role == Qt::DisplayRole)
		{
			bt::TorrentInterface* tc = torrents[index.row()];
			switch (index.column())
			{
				case 0: return tc->getStats().torrent_name;
				case 1: 
					if (tc->getPriority() == 0)
						return i18n("Not queued");
					else if (tc->getStats().running)
						return i18n("Running");
					else
						return i18n("Queued");
					break;
				case 2: return tc->getPriority();
				default: return QVariant();
			}
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags QueueManagerModel::flags(const QModelIndex &index) const
	{
		Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

		if (index.isValid())
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		else
			return Qt::ItemIsDropEnabled | defaultFlags;
	}
	
	Qt::DropActions QueueManagerModel::supportedDropActions() const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}
	
	QStringList QueueManagerModel::mimeTypes() const
	{
		QStringList types;
		types << "application/vnd.text.list";
		return types;
	}
					
	QMimeData* QueueManagerModel::mimeData(const QModelIndexList &indexes) const
	{
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;

		dragged_items.clear();
	
		foreach (QModelIndex index, indexes) 
		{
			if (index.isValid() && !dragged_items.contains(index.row())) 
				dragged_items.append(index.row());
		}
		
		mimeData->setData("application/vnd.text.list", "stuff");
		return mimeData;
	}

	bool QueueManagerModel::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)		
	{
		if (action == Qt::IgnoreAction)
			return true;

		if (!data->hasFormat("application/vnd.text.list"))
			return false;
		
		int begin_row;
		if (row != -1)
			begin_row = row;
		else if (parent.isValid())
			begin_row = parent.row();
		else
			begin_row = rowCount(QModelIndex());
		
		// put the dragged ones in a new list
		QList<bt::TorrentInterface*> tcs;
		foreach (int r,dragged_items)
		{
			bt::TorrentInterface* tc = torrents.at(r);
			if (tc)
			{
				tcs.append(tc);
				if (r < begin_row) // begin row will decrease when we remove this one in the next loop
					begin_row--;
				
				if (tc->getPriority() == 0) // once you drag items they become QM controlled
					tc->setPriority(1);
			}
		}
		
		// remove the dragged ones
		foreach (bt::TorrentInterface* tc,tcs)
			torrents.removeAll(tc);
		
		// reinsert them at the correct location
		foreach (bt::TorrentInterface* tc,tcs)
			torrents.insert(begin_row,tc);
		
		int prio = torrents.count();
		// redo the priorities
		foreach (bt::TorrentInterface* t,torrents)
		{
			if (t->getPriority() > 0)
				t->setPriority(prio);
			prio--;
		}
		
		qman->orderQueue();
		return true;
	}

	bool QueueManagerModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
			torrents.takeAt(row);
		endInsertRows();
		return true;
	}
	
	bool QueueManagerModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	void QueueManagerModel::moveUp(int row)
	{
		if (row <= 0 || row > torrents.count())
			return;
		
		int prio = torrents.count();
		bt::TorrentInterface* tc = torrents.takeAt(row);
		torrents.insert(row - 1,tc);
		
		// redo the priorities
		foreach (bt::TorrentInterface* t,torrents)
		{
			if (t->getPriority() > 0 || tc == t)
				t->setPriority(prio);
			prio--;
		}
		// reorder the queue
		qman->orderQueue();
	}
	
	void QueueManagerModel::moveDown(int row)
	{
		if (row < 0 || row >= torrents.count() - 1)
			return;
		
		int prio = torrents.count();
		bt::TorrentInterface* tc = torrents.takeAt(row);
		torrents.insert(row + 1,tc);
		
		// redo the priorities
		foreach (bt::TorrentInterface* t,torrents)
		{
			if (t->getPriority() > 0 || tc == t)
				t->setPriority(prio);
			prio--;
		}
		// reorder the queue
		qman->orderQueue();
	}
	
	void QueueManagerModel::queue(int row)
	{
		if (row < 0 || row >= torrents.count())
			return;
		
		bt::TorrentInterface* tc = torrents[row];
		qman->queue(tc);
		qman->orderQueue();
	}
}

#include "queuemanagermodel.moc"