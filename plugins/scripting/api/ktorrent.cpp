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
#include <util/log.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include "ktorrent.h"
#include "torrent.h"

using namespace bt;

namespace ktapi
{

	KTorrent::KTorrent(kt::CoreInterface* core,QObject* parent)
			: QObject(parent),core(core)
	{
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(torrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(torrentRemoved(bt::TorrentInterface*)));
		
		kt::QueueManager* qman = core->getQueueManager();
		for (kt::QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrentAdded(*i);
	}


	KTorrent::~KTorrent()
	{
	}
	
	void KTorrent::torrentAdded(bt::TorrentInterface* ti)
	{
		torrents.append(new ktapi::Torrent(ti,this));
	}
	
	void KTorrent::torrentRemoved(bt::TorrentInterface* ti)
	{
		foreach (ktapi::Torrent* t,torrents)
		{
			if (*t == ti)
			{
				torrents.removeAll(t);
				delete t;
				return;
			}
		}
	}
	
	void KTorrent::log(const QString & str)
	{
		Out(SYS_SCR|LOG_NOTICE) << str << endl;
	}
	
	int KTorrent::numTorrents()
	{
		return torrents.count();
	}
	
	QObject* KTorrent::torrent(int i)
	{
		if (i >= 0 && i < torrents.count())
			return torrents[i];
		else
			return 0;
	}

}
