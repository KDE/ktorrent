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
#include <klocale.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "queuemanagermodel.h"

namespace kt
{

	QueueManagerModel::QueueManagerModel(Filter filter,QueueManager* qman,QObject* parent)
			: QAbstractTableModel(parent),qman(qman),filter(filter)
	{
		QList<bt::TorrentInterface*>::iterator i = qman->begin();
		while (i != qman->end())
		{
			bt::TorrentInterface* tc = *i;
			connect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),this,SLOT(onStatusChanged(bt::TorrentInterface*)));
			connect(tc,SIGNAL(finished(bt::TorrentInterface*)),this,SLOT(onTorrentFinished(bt::TorrentInterface*)));
			if (filter == DOWNLOADS && !tc->getStats().completed)
				torrents.append(tc);
			else if (filter == UPLOADS && tc->getStats().completed)
				torrents.append(tc);
			i++;
		}
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
	
	void QueueManagerModel::checkTorrentMemberShip()
	{
		QList<bt::TorrentInterface*> misfits;
		foreach (bt::TorrentInterface* tc,torrents)
		{
			if ((filter == DOWNLOADS && tc->getStats().completed) || (filter == UPLOADS && !tc->getStats().completed))
				misfits.append(tc);
		}
		
		foreach (bt::TorrentInterface* tc,misfits)
			onTorrentRemoved(tc);
	}
	
	void QueueManagerModel::onStatusChanged(bt::TorrentInterface* tc)
	{
		int idx = torrents.indexOf(tc);
		if (idx >= 0)
		{
			torrents.sort();
			int nidx = torrents.indexOf(tc);
			if (nidx == idx)
				dataChanged(createIndex(idx,0),createIndex(idx,2));
			else
				reset(); // order changed, so reset the model
		}
		checkTorrentMemberShip();
	}
	
	void QueueManagerModel::onTorrentFinished(bt::TorrentInterface* tc)
	{
		if (filter == UPLOADS)
		{
			torrents.append(tc);
			insertRow(torrents.count() - 1);
			torrents.sort();
			reset();
		}
		else
			checkTorrentMemberShip(); // check if we need to drop them
	}
	
	void QueueManagerModel::onTorrentAdded(bt::TorrentInterface* tc)
	{
		connect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),this,SLOT(onStatusChanged(bt::TorrentInterface*)));
		connect(tc,SIGNAL(finished(bt::TorrentInterface*)),this,SLOT(onTorrentFinished(bt::TorrentInterface*)));
		if ((filter == DOWNLOADS && !tc->getStats().completed) || (filter == UPLOADS && tc->getStats().completed))
		{
			torrents.append(tc);
			insertRow(torrents.count() - 1);
			torrents.sort();
			reset();
		}
	}
	
	void QueueManagerModel::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		int r = torrents.indexOf(tc);
		if (r >= 0 && r < torrents.count())
			removeRow(r);
	}

	int QueueManagerModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return filter == DOWNLOADS ? qman->countDownloads() : qman->countSeeds();
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
		if (row < 0 || row > torrents.count() - 1)
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
}

#include "queuemanagermodel.moc"