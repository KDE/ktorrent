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
#include <QDBusConnection>
#include <kconfig.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <util/log.h>
#include "dbus.h"
#include "core.h"
#include "gui.h"

using namespace bt;

namespace kt
{
	DBus::DBus(GUI* gui,Core* core) : QObject(gui),gui(gui),core(core)
	{
		QDBusConnection::sessionBus().registerObject("/KTorrent", this,
				QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);

		connect(core,SIGNAL(torrentAdded(kt::TorrentInterface*)),this,SLOT(torrentAdded(kt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(kt::TorrentInterface*)),this,SLOT(torrentAdded(kt::TorrentInterface*)));
		// fill the map with torrents
		torrents();
	}

	DBus::~DBus()
	{
	}

	QStringList DBus::torrents()
	{
		bt::QueueManager* qm = core->getQueueManager();
		QStringList tors;
		for (QList<kt::TorrentInterface *>::iterator i = qm->begin();i != qm->end();i++)
		{
			kt::TorrentInterface* tc = *i;
			tors.append(tc->getStats().torrent_name);
			if (!torrent_map.contains(tc->getStats().torrent_name))
				torrent_map.insert(tc->getStats().torrent_name,tc);
		}

		return tors;
	}
	
	void DBus::start(const QString & torrent)
	{
		kt::TorrentInterface* tc = torrent_map.find(torrent);
		if (!tc)
			return;

		core->getQueueManager()->start(tc,true);
	}

	void DBus::stop(const QString & torrent)
	{
		kt::TorrentInterface* tc = torrent_map.find(torrent);
		if (!tc)
			return;

		core->getQueueManager()->stop(tc,true);
	}

	void DBus::startAll()
	{
		core->startAll(3);
	}

	void DBus::stopAll()
	{
		core->stopAll(3);
	}

	int DBus::downloadSpeed(const QString & torrent)
	{
		kt::TorrentInterface* tc = torrent_map.find(torrent);
		if (!tc)
			return -1;

		return tc->getStats().download_rate;
	}

	int DBus::uploadSpeed(const QString & torrent)
	{
		kt::TorrentInterface* tc = torrent_map.find(torrent);
		if (!tc)
			return -1;

		return tc->getStats().upload_rate;
	}
	
	void DBus::torrentAdded(kt::TorrentInterface* tc)
	{
		torrent_map.insert(tc->getStats().torrent_name,tc);
		torrentAdded(tc->getStats().torrent_name);
	}

	void DBus::torrentRemoved(kt::TorrentInterface* tc)
	{
		torrent_map.erase(tc->getStats().torrent_name);
		torrentRemoved(tc->getStats().torrent_name);
	}

}

#include "dbus.moc"
