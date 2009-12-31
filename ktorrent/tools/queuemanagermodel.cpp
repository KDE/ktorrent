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
#include <KIcon>
#include <KLocale>
#include <util/log.h>
#include <util/functions.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "queuemanagermodel.h"
#include "settings.h"
#include <QApplication>


using namespace bt;

namespace kt
{


	QueueManagerModel::QueueManagerModel(QueueManager* qman,QObject* parent)
			: QAbstractTableModel(parent),qman(qman)
	{
		connect(qman,SIGNAL(queueOrdered()),this,SLOT(onQueueOrdered()));
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* tc = *i;
			connect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),this,SLOT(onTorrentStatusChanged(bt::TorrentInterface*)));
		}
	}


	QueueManagerModel::~QueueManagerModel()
	{}
	
	void QueueManagerModel::onQueueOrdered()
	{
		reset();
	}
	
	void QueueManagerModel::onTorrentAdded(bt::TorrentInterface* tc)
	{
		insertRow(qman->count() - 1);
		stalled_times.insert(tc,0);
		connect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),this,SLOT(onTorrentStatusChanged(bt::TorrentInterface*)));
	}
	
	void QueueManagerModel::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		disconnect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),this,SLOT(onTorrentStatusChanged(bt::TorrentInterface*)));
		int r = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			if (tc == *i)
			{
				stalled_times.remove(tc);
				removeRow(r);
				break;
			}
			r++;
		}
	}
	
	void QueueManagerModel::onTorrentStatusChanged(bt::TorrentInterface* tc)
	{
		int r = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			if (tc == *i)
			{
				QModelIndex idx = index(r,2);
				emit dataChanged(idx,idx);
				break;
			}
			r++;
		}
	}

	int QueueManagerModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return qman->count();
	}
	
	int QueueManagerModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 4;
	}
	
	QVariant QueueManagerModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Order");
			case 1: return i18n("Name");
			case 2: return i18n("Status");
			case 3: return i18n("Time Stalled");
			case 4: return i18n("Priority");
			default: return QVariant();
		}
	}
	
	QVariant QueueManagerModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid() || index.row() >= qman->count() || index.row() < 0)
			return QVariant();
		
		if (role == Qt::ForegroundRole)
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			if (index.column() == 2)
			{
				if (tc->getStats().running)
					return QColor(40,205,40); // green
				else if (tc->getStats().status == bt::QUEUED)
					return QColor(255,174,0); // yellow
				else
					return QVariant();
			}
			return QVariant();
		}
		else if (role == Qt::DisplayRole)
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			switch (index.column())
			{
				case 0: return index.row() + 1;
				case 1: return tc->getDisplayName();
				case 2: 
					if (tc->getStats().running)
						return i18n("Running");
					else if (tc->getStats().status == bt::QUEUED)
						return i18n("Queued");
					else
						return i18n("Not queued");
					break;
				case 3: 
					{
						if (!tc->getStats().running)
							return QVariant();
						
						Int64 stalled_time = stalled_times.value(tc);
						if (stalled_time >= 1)
							return i18n("%1",DurationToString(stalled_time));
						else
							return QVariant();
					}
					break;
				case 4:
					return tc->getPriority();
				default: return QVariant();
			}
		}
		else if (role == Qt::ToolTipRole && index.column() == 0)
		{
			return i18n("Order of a torrent in the queue.\n"
					"Use drag and drop or the move up and down buttons on the right to change the order.");
		}
		else if (role == Qt::DecorationRole && index.column() == 1)
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			if (!tc->getStats().completed)
				return KIcon("arrow-down");
			else
				return KIcon("arrow-up");
		}
		else if (role == Qt::FontRole && !search_text.isEmpty())
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			QFont f = QApplication::font();
			if (tc->getDisplayName().contains(search_text,Qt::CaseInsensitive))
				f.setBold(true);
			
			return f;
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
	
		foreach (const QModelIndex &index, indexes) 
		{
			if (index.isValid() && !dragged_items.contains(index.row())) 
				dragged_items.append(index.row());
		}
		
		mimeData->setData("application/vnd.text.list", "stuff");
		return mimeData;
	}

	bool QueueManagerModel::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)		
	{
		Q_UNUSED(column);
		if (action == Qt::IgnoreAction)
			return true;

		if (!data->hasFormat("application/vnd.text.list"))
			return false;
		
		int begin_row = row;
		if (row != -1)
			begin_row = row;
		else if (parent.isValid())
			begin_row = parent.row();
		else
			begin_row = rowCount(QModelIndex());
		
		
		// use a copy of the list of torrents to redo the priorities
		QueuePtrList torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		int dragged_row = dragged_items.front();
		int count = dragged_items.count();
		
		QList<bt::TorrentInterface*> middle;
		for (int i = 0;i < count;i++)
		{
			middle.append(torrents.takeAt(dragged_row));
			if (begin_row >= dragged_row && begin_row > 0)
				begin_row--;
		}
		
		int cnt = 0;
		foreach (bt::TorrentInterface* tc,middle)
		{
			torrents.insert(begin_row + cnt,tc);
			cnt++;
		}
		
		int prio = torrents.count();
		//redo priorites
		foreach (bt::TorrentInterface* tc,torrents)
		{
			tc->setPriority(prio--);
		}
		
		// reorder the queue
		qman->orderQueue();
		reset();
		return true;
	}

	bool QueueManagerModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool QueueManagerModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	void QueueManagerModel::moveUp(int row,int count)
	{
		if (row <= 0 || row > qman->count())
			return;
			
		QList<bt::TorrentInterface*> torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		for (int i = row;i < row + count;i++)
			torrents.swap(i,i - 1);
			
		int prio = torrents.count();
		//redo priorites
		foreach (bt::TorrentInterface* tc,torrents)
		{
			tc->setPriority(prio--);
		}
		
		// reorder the queue
		qman->orderQueue();
		reset();
	}
	
	void QueueManagerModel::moveDown(int row,int count)
	{
		if (row < 0 || row >= qman->count() - 1)
			return;
		
		QList<bt::TorrentInterface*> torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		QList<bt::TorrentInterface*> middle;
		for (int i = 0;i < count;i++)
			middle.append(torrents.takeAt(row));
			
		int cnt = 0;
		foreach (bt::TorrentInterface* tc,middle)
		{
			torrents.insert(row + 1 + cnt,tc);
			cnt++;
		}
		
		int prio = torrents.count();
		//redo priorites
		foreach (bt::TorrentInterface* tc,torrents)
		{
			tc->setPriority(prio--);
		}
		
		// reorder the queue
		qman->orderQueue();
		reset();
	}
	
	void QueueManagerModel::moveTop(int row,int count)
	{
		if (row < 0 || row >= qman->count())
			return;
		
		QList<bt::TorrentInterface*> torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		QList<bt::TorrentInterface*> front;
		for (int i = 0;i < count;i++)
			front.append(torrents.takeAt(row));
		
		torrents = front + torrents;
			
		int prio = torrents.count();
		//redo priorites
		foreach (bt::TorrentInterface* tc,torrents)
		{
			tc->setPriority(prio--);
		}
		// reorder the queue
		qman->orderQueue();
		reset();
	}
	
	void QueueManagerModel::moveBottom(int row,int count)
	{
		if (row < 0 || row >= qman->count())
			return;
		
		QList<bt::TorrentInterface*> torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		QList<bt::TorrentInterface*> back;
		for (int i = 0;i < count;i++)
			back.append(torrents.takeAt(row));
		
		torrents = torrents + back;
			
		int prio = torrents.count();
		//redo priorites
		foreach (bt::TorrentInterface* tc,torrents)
		{
			tc->setPriority(prio--);
		}
		// reorder the queue
		qman->orderQueue();
		reset();
	}
	
	void QueueManagerModel::update()
	{
		TimeStamp now = bt::GetCurrentTime();
		int r = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* tc = *i;
			if (!tc->getStats().running)
			{
				if (stalled_times[tc] != -1)
				{
					stalled_times[tc] = -1;
					emit dataChanged(createIndex(r,3),createIndex(r,3));
				}
			}
			else
			{
				Int64 stalled_time = 0;
				if (tc->getStats().completed)
					stalled_time = (now - tc->getStats().last_upload_activity_time) / 1000;
				else
					stalled_time = (now - tc->getStats().last_download_activity_time) / 1000;
				
				if (stalled_times[tc] != stalled_time)
				{
					stalled_times[tc] = stalled_time;
					emit dataChanged(createIndex(r,3),createIndex(r,3));
				}
			}
			r++;
		}
	}
	
	
	QModelIndex QueueManagerModel::find(const QString& text)
	{
		search_text = text;
		if (text.isEmpty())
		{
			reset();
			return QModelIndex();
		}
		
		int idx = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* tc = *i;
			if (tc->getDisplayName().contains(text,Qt::CaseInsensitive))
			{
				reset();
				return index(idx,0);
			}
			idx++;
		}
		
		reset();
		return QModelIndex();
	}

}

#include "queuemanagermodel.moc"
