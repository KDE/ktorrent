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
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <kicon.h>
#include <klocale.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include "feed.h"
#include "feedlist.h"
#include "filterlist.h"

using namespace bt;

namespace kt
{

	FeedList::FeedList(const QString & data_dir,QObject* parent) : QAbstractListModel(parent),data_dir(data_dir)
	{
	}


	FeedList::~FeedList()
	{
		qDeleteAll(feeds);
	}
	
	void FeedList::loadFeeds(FilterList* filter_list)
	{
		QDir dir(data_dir);
		QStringList filters;
		filters << "feed*";
		QStringList sl = dir.entryList(filters,QDir::Dirs);
		for (int i = 0;i < sl.count();i++)
		{
			QString idir = data_dir + sl.at(i);
			if (!idir.endsWith(DirSeparator()))
				idir.append(DirSeparator());

			Out(SYS_GEN|LOG_NOTICE) << "Loading feed from directory " << idir << endl;
			Feed* feed = 0;
			try
			{
				feed = new Feed(idir);
				feed->load(filter_list);
				feed->refresh();
			}
			catch (...)
			{
				delete feed;
			}
			addFeed(feed);
		}
	}
	
	void FeedList::addFeed(Feed* f)
	{
		feeds.append(f);
		connect(f,SIGNAL(updated()),this,SLOT(feedUpdated()));
		insertRow(feeds.count() - 1);
	}

	int FeedList::rowCount(const QModelIndex & parent) const
	{
		return !parent.isValid() ? feeds.count() : 0;
	}
	
	QVariant FeedList::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
		
		Feed* f = feeds.at(index.row());
		switch (role)
		{
			case Qt::DisplayRole:
				if (f->ok())
					return f->feedData()->title();
				else
					return f->feedUrl().prettyUrl();
			case Qt::DecorationRole:
				return KIcon("application-rss+xml");
			case Qt::ToolTipRole:
				if (f->ok())
					return i18n("<b>%1</b><br/><br/>%2",f->feedData()->link(),f->feedData()->description());
				break;
		}
		
		return QVariant();
	}
	
	Feed* FeedList::feedForIndex(const QModelIndex & idx)
	{
		if (!idx.isValid())
			return 0;
		
		return feeds.at(idx.row());
	}
	
	Feed* FeedList::feedForDirectory(const QString & dir)
	{
		foreach (Feed* f,feeds)
			if (f->directory() == dir)
				return f;
		
		return 0;
	}
	
	bool FeedList::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool FeedList::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	void FeedList::removeFeeds(const QModelIndexList & idx)
	{
		QList<Feed*> to_remove;
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feedForIndex(i);
			if (f)
				to_remove.append(f);
		}
		
		foreach (Feed* f,to_remove)
		{
			bt::Delete(f->directory(),true);
			feeds.removeAll(f);
			delete f;
		}
		
		reset();
	}
	
	void FeedList::feedUpdated()
	{
		Feed* f = (Feed*)sender();
		int idx = feeds.indexOf(f);
		if (idx >= 0)
			emit dataChanged(index(idx,0),index(idx,0));
	}
	
	void FeedList::filterRemoved(Filter* f)
	{
		foreach (Feed* feed,feeds)
			feed->removeFilter(f);
	}
}
