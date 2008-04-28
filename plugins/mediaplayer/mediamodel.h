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
#ifndef KTMEDIAMODEL_H
#define KTMEDIAMODEL_H

#include <QAbstractItemModel>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class CoreInterface;

	/**
		@author
	*/
	class MediaModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		MediaModel(CoreInterface* core,QObject* parent);
		virtual ~MediaModel();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual QModelIndex index(int row,int column,const QModelIndex & parent) const;
		virtual QModelIndex parent(const QModelIndex & index) const;
		
		/// Get the full path of the model index
		QString pathForIndex(const QModelIndex & idx) const;
		
		/// Get the index of a full path
		QModelIndex indexForPath(const QString & path) const;
		
		/// Get the next item to play, if idx is invalid return the first playable item
		QModelIndex next(const QModelIndex & idx,bool random,bool complete_only) const;
		
	public slots:
		void onTorrentAdded(bt::TorrentInterface* t);
		void onTorrentRemoved(bt::TorrentInterface* t);
		
	private:
		QModelIndex next(const QModelIndex & idx) const;
		QModelIndex randomNext(const QModelIndex & idx,bool complete_only) const;
		
	private:
		struct Item
		{
			bt::TorrentInterface* tc;
			QList<int> multimedia_files;
			
			Item(bt::TorrentInterface* tc);
		};
		CoreInterface* core;
		QList<Item*> items;
		bt::Uint32 total_number_of_media_files;
	};

}

#endif
