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
#include <QList>
#include <interfaces/torrentinterface.h>
#include "trackermodel.h"

namespace kt
{

	TrackerModel::TrackerModel(QObject* parent)
			: QAbstractListModel(parent),tc(0)
	{
	}


	TrackerModel::~TrackerModel()
	{
	}

	void TrackerModel::changeTC(bt::TorrentInterface* tc)
	{
		trackers.clear();
		this->tc = tc;
		if (tc)
			trackers = tc->getTrackersList()->getTrackerURLs();
		
		reset();
	}

	int TrackerModel::rowCount(const QModelIndex &parent) const
	{
		if (parent.isValid() || !tc)
			return 0;
		else
			return trackers.count();
	}
	
	QVariant TrackerModel::data(const QModelIndex &index, int role) const
	{
		if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
			return QVariant();
		
		KUrl url = trackers.at(index.row());
		if (role == Qt::DisplayRole)
			return url.prettyUrl();
		else if (role == Qt::CheckStateRole)
			return tc->getTrackersList()->isTrackerEnabled(url) ? Qt::Checked : Qt::Unchecked;
		else
			return QVariant();
	}
	
	bool TrackerModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!tc || !index.isValid() || index.row() < 0 || index.row() >= trackers.count())
			return false;
		
		if (role == Qt::CheckStateRole)
		{
			KUrl url = trackers.at(index.row());
			tc->getTrackersList()->setTrackerEnabled(url,(Qt::CheckState)value.toUInt() == Qt::Checked);
			return true;
		}
		return false;
	}
	
	QVariant TrackerModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		Q_UNUSED(section);
		Q_UNUSED(orientation);
		Q_UNUSED(role);
		return QVariant();
	}
	
	bool TrackerModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		if (tc)
			trackers = tc->getTrackersList()->getTrackerURLs();
		endInsertRows();
		return true;
	}
	
	bool TrackerModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		if (tc)
			trackers = tc->getTrackersList()->getTrackerURLs();
		endRemoveRows();
		return true;
	}
	
	Qt::ItemFlags TrackerModel::flags(const QModelIndex & index) const
	{
		if (!tc || !index.isValid() || index.row() >= trackers.count() || index.row() < 0)
			return QAbstractItemModel::flags(index);
		else
			return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
	}
	
	bool TrackerModel::hasTracker(const KUrl & url) const
	{
		return trackers.indexOf(url) != -1;
	}
	
	KUrl TrackerModel::trackerUrl(const QModelIndex & index)
	{
		if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
			return KUrl();
		
		return trackers.at(index.row());
	}
}
