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

#include <KUrl>
#include <QAbstractListModel>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{

	/**
		@author
	*/
	class TrackerModel : public QAbstractListModel
	{
		Q_OBJECT
	public:
		TrackerModel(QObject* parent);
		virtual ~TrackerModel();
		
		void changeTC(bt::TorrentInterface* tc);

		virtual int rowCount(const QModelIndex &parent) const;
		virtual QVariant data(const QModelIndex &index, int role) const;
		virtual bool setData(const QModelIndex & index,const QVariant & value,int role);
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		
		/// Check if the model contains a tracker
		bool hasTracker(const KUrl & url) const;
		
		/// Get a tracker url given a model index
		KUrl trackerUrl(const QModelIndex & idx);
		
	private:
		bt::TorrentInterface* tc;
		KUrl::List trackers;
	};

}

#endif
