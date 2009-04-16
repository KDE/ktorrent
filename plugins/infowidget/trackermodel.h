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
#ifndef KTTRACKERMODEL_H
#define KTTRACKERMODEL_H

#include <QList>
#include <KUrl>
#include <QAbstractTableModel>
#include <interfaces/trackerinterface.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{

	/**
		@author
	*/
	class TrackerModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		TrackerModel(QObject* parent);
		virtual ~TrackerModel();
		
		void changeTC(bt::TorrentInterface* tc);
		void update();

		virtual int rowCount(const QModelIndex &parent) const;
		virtual int columnCount(const QModelIndex &parent) const;
		virtual QVariant data(const QModelIndex &index, int role) const;
		virtual bool setData(const QModelIndex & index,const QVariant & value,int role);
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		virtual QModelIndex index(int row,int column,const QModelIndex & parent = QModelIndex()) const;
		
		/// Get a tracker url given a model index
		KUrl trackerUrl(const QModelIndex & idx);
		
		/// Get a tracker given a model index
		bt::TrackerInterface* tracker(const QModelIndex & idx);
		
	private:
		struct Item
		{
			bt::TrackerInterface* trk;
			bt::TrackerStatus status;
			int seeders;
			int leechers;
			int times_downloaded;
			int time_to_next_update;
			
			Item(bt::TrackerInterface* tracker);
			bool update();
			QVariant displayData(int column) const;
			QVariant sortData(int column) const;
		};
		
		bt::TorrentInterface* tc;
		QList<Item*> trackers;
		bool running;
	};

}

#endif
