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
#ifndef KTQUEUEMANAGERMODEL_H
#define KTQUEUEMANAGERMODEL_H

#include <QList>
#include <QAbstractTableModel>
#include <torrent/queuemanager.h>

class QMimeData;

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class QueueManager;

	/**
	 * @author Joris Guisson
	 * 
	 * Model for the QM
	*/
	class QueueManagerModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		QueueManagerModel(QueueManager* qman,QObject* parent);
		virtual ~QueueManagerModel();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual Qt::ItemFlags flags(const QModelIndex &index) const;
		virtual Qt::DropActions supportedDropActions() const;
		virtual QStringList mimeTypes() const;
		virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
		virtual bool dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent);
		
		/**
		 * Move an item one row up
		 * @param row The row of the item
		 */
		void moveUp(int row);
		
		/**
		 * Move an item one row down
		 * @param row The row of the item
		 */
		void moveDown(int row);
		
		/**
		 * Enqueue or dequeue an item
		 * @param row The row of the item
		 */
		void queue(int row);
		
		/**
		 * Update the model
		 */
		void update();
		
	public slots:
		void onTorrentAdded(bt::TorrentInterface* tc);
		void onTorrentRemoved(bt::TorrentInterface* tc);
		void onQueueOrdered();

	private:
		QueueManager* qman;
		QMap<const bt::TorrentInterface*,bt::Int64> stalled_times;
		mutable QList<int> dragged_items;
	};

}

#endif
