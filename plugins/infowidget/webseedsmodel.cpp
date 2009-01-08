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
#include "webseedsmodel.h"
#include <klocale.h>
#include <interfaces/webseedinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{

	WebSeedsModel::WebSeedsModel(QObject* parent)
			: QAbstractTableModel(parent),curr_tc(0)
	{
	}


	WebSeedsModel::~WebSeedsModel()
	{
	}

	void WebSeedsModel::changeTC(bt::TorrentInterface* tc)
	{
		curr_tc = tc;
		items.clear();
		if (tc)
		{
			for (Uint32 i = 0;i < tc->getNumWebSeeds();i++)
			{
				const bt::WebSeedInterface* ws = curr_tc->getWebSeed(i);
				Item item;
				item.status = ws->getStatus();
				item.downloaded = ws->getTotalDownloaded();
				item.speed = ws->getDownloadRate();
				items.append(item);
			}
		}
		reset();
	}
	
	bool WebSeedsModel::update()
	{
		if (!curr_tc)
			return false;
		
		bool ret = false;
		
		for (Uint32 i = 0;i < curr_tc->getNumWebSeeds();i++)
		{
			const bt::WebSeedInterface* ws = curr_tc->getWebSeed(i);
			Item & item = items[i];
			bool changed = false;
			if (item.status != ws->getStatus())
			{
				changed = true;
				item.status = ws->getStatus();
			}
				
			if (item.downloaded != ws->getTotalDownloaded())
			{
				changed = true;
				item.downloaded = ws->getTotalDownloaded();
			}
			
			if (item.speed != ws->getDownloadRate())
			{
				changed = true;
				item.speed = ws->getDownloadRate();
			}
			
			if (changed)
			{
				dataChanged(createIndex(i,1),createIndex(i,3));
				ret = true;
			}
		}
		
		return ret;
	}
		
	int WebSeedsModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return curr_tc ? curr_tc->getNumWebSeeds() : 0;
	}
	
	int WebSeedsModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 4;
	}
	
	QVariant WebSeedsModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("URL");
			case 1: return i18n("Speed");
			case 2: return i18n("Downloaded");
			case 3: return i18n("Status");
			default: return QVariant();
		}
	}
	
	QVariant WebSeedsModel::data(const QModelIndex & index, int role) const
	{
		if (!curr_tc)
			return QVariant();
		
		if (!index.isValid() || index.row() >= curr_tc->getNumWebSeeds() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::DisplayRole)
		{
			const bt::WebSeedInterface* ws = curr_tc->getWebSeed(index.row());
			switch (index.column())
			{
				case 0: return ws->getUrl().prettyUrl();
				case 1: return bt::BytesPerSecToString(ws->getDownloadRate());
				case 2: return bt::BytesToString(ws->getTotalDownloaded());
				case 3: return ws->getStatus();
			}
		}
		return QVariant();
	}

}
