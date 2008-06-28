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
#ifndef KT_DBUS_HH
#define KT_DBUS_HH

#include <QObject>
#include <util/ptrmap.h>

namespace kt
{
	class GUI;
	class Core;
	class DBusTorrent;
	class DBusGroup;
	

	/**
	 * Class which handles DBus calls
	 * */
	class DBus : public QObject
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.ktorrent.core")
	public:
		DBus(GUI* gui,Core* core);
		virtual ~DBus();

	public Q_SLOTS:
		/// Get the names of all torrents
		Q_SCRIPTABLE QStringList torrents();

		/// Start a torrent 
		Q_SCRIPTABLE void start(const QString & info_hash);

		/// Stop a torrent
		Q_SCRIPTABLE void stop(const QString & info_hash);

		/// Start all torrents
		Q_SCRIPTABLE void startAll();

		/// Stop all torrents
		Q_SCRIPTABLE void stopAll();
		
		/// Load a torrent
		Q_SCRIPTABLE void load(const QString & url,const QString & group);
		
		/// Load a torrent silently
		Q_SCRIPTABLE void loadSilently(const QString & url,const QString & group);
		
		/// Get all the custom groups
		Q_SCRIPTABLE QStringList groups() const;
		
		/// Add a group
		Q_SCRIPTABLE bool addGroup(const QString & group);

		/// Remove a group
		Q_SCRIPTABLE bool removeGroup(const QString & group);
		
	private Q_SLOTS:
		void torrentAdded(bt::TorrentInterface* tc);
		void torrentRemoved(bt::TorrentInterface* tc);
		void groupAdded(Group* g);
		void groupRemoved(Group* g);

	 Q_SIGNALS: 
		/// DBus signal emitted when a torrent has been added
		Q_SCRIPTABLE void torrentAdded(const QString & tor);

		/// DBus signal emitted when a torrent has been removed
		Q_SCRIPTABLE void torrentRemoved(const QString & tor);
		

	private:
		GUI* gui;
		Core* core;
		bt::PtrMap<QString,DBusTorrent> torrent_map;
		bt::PtrMap<Group*,DBusGroup> group_map;
		
		typedef bt::PtrMap<QString,DBusTorrent>::iterator DBusTorrentItr;
		typedef bt::PtrMap<Group*,DBusGroup>::iterator DBusGroupItr;
	};

}

#endif
