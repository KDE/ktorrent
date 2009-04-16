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
#include "trackermodel.h"
#include <QList>
#include <klocale.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerinterface.h>

namespace kt
{

	TrackerModel::TrackerModel(QObject* parent)
			: QAbstractTableModel(parent),tc(0)
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
			trackers = tc->getTrackersList()->getTrackers();
		
		reset();
	}

	int TrackerModel::rowCount(const QModelIndex &parent) const
	{
		if (parent.isValid() || !tc)
			return 0;
		else
			return trackers.count();
	}
	
	int TrackerModel::columnCount(const QModelIndex& parent) const 
	{
		if (parent.isValid())
			return 0;
		else
			return 6;
	}

	
	QVariant TrackerModel::data(const QModelIndex &index, int role) const
	{
		if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
			return QVariant();
		
		bt::TrackerInterface* trk = trackers.at(index.row());
		if (role == Qt::CheckStateRole && index.column() == 0)
			return trk->isEnabled() ? Qt::Checked : Qt::Unchecked;
		
		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
			case 0: return trk->trackerURL().prettyUrl();
			case 1: return trk->trackerStatusString();
			case 2: return trk->getNumSeeders();
			case 3: return trk->getNumLeechers();
			case 4: return trk->getTotalTimesDownloaded();
			case 5: return QTime().addSecs(trk->timeToNextUpdate()).toString("mm:ss");
			}
			
		}
		
		return QVariant();
	}
	
	bool TrackerModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!tc || !index.isValid() || index.row() < 0 || index.row() >= trackers.count())
			return false;
		
		if (role == Qt::CheckStateRole)
		{
			KUrl url = trackers.at(index.row())->trackerURL();
			tc->getTrackersList()->setTrackerEnabled(url,(Qt::CheckState)value.toUInt() == Qt::Checked);
			return true;
		}
		return false;
	}
	
	QVariant TrackerModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (orientation != Qt::Horizontal)
			return QVariant();
		
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
				case 0: return i18n("Url");
				case 1: return i18n("Status");
				case 2: return i18n("Seeders");
				case 3: return i18n("Leechers");
				case 4: return i18n("Times Downloaded");
				case 5: return i18n("Next Update");
			}
		}
		return QVariant();
	}
	
	bool TrackerModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		if (tc)
			trackers = tc->getTrackersList()->getTrackers();
		endInsertRows();
		return true;
	}
	
	bool TrackerModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		if (tc)
			trackers = tc->getTrackersList()->getTrackers();
		endRemoveRows();
		return true;
	}
	
	Qt::ItemFlags TrackerModel::flags(const QModelIndex & index) const
	{
		if (!tc || !index.isValid() || index.row() >= trackers.count() || index.row() < 0 || index.column() != 0)
			return QAbstractItemModel::flags(index);
		else
			return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
	}
	
	KUrl TrackerModel::trackerUrl(const QModelIndex & index)
	{
		if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
			return KUrl();
		
		return trackers.at(index.row())->trackerURL();
	}
	
	bt::TrackerInterface* TrackerModel::tracker(const QModelIndex & index) 
	{
		if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
			return 0;
		
		return trackers.at(index.row());
	}

}
