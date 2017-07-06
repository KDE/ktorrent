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

#include <QMap>
#include <QObject>
#include <QStringList>

#include <ktcore_export.h>
#include <util/ptrmap.h>
#include <dbus/dbusgroup.h>
#include <dbus/dbustorrent.h>


namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    class GUIInterface;
    class CoreInterface;
    class Group;
    class DBusSettings;

    /**
     * Class which handles DBus calls
     * */
    class KTCORE_EXPORT DBus : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.ktorrent.core")
    public:
        DBus(GUIInterface* gui, CoreInterface* core, QObject* parent);
        ~DBus();


    public Q_SLOTS:
        /// Get the names of all torrents
        Q_SCRIPTABLE QStringList torrents();

        /// Start a torrent
        Q_SCRIPTABLE void start(const QString& info_hash);

        /// Stop a torrent
        Q_SCRIPTABLE void stop(const QString& info_hash);

        /// Start all torrents
        Q_SCRIPTABLE void startAll();

        /// Stop all torrents
        Q_SCRIPTABLE void stopAll();

        /// Load a torrent
        Q_SCRIPTABLE void load(const QString& url, const QString& group);

        /// Load a torrent silently
        Q_SCRIPTABLE void loadSilently(const QString& url, const QString& group);

        /// Remove a torrent
        Q_SCRIPTABLE void remove(const QString& info_hash, bool data_to);

        /// Remove a torrent delayed (should be used from signal handlers)
        Q_SCRIPTABLE void removeDelayed(const QString& info_hash, bool data_to);

        /// Set the suspended state
        Q_SCRIPTABLE void setSuspended(bool suspend);

        /// Gets the globla suspended state
        Q_SCRIPTABLE bool suspended();

        /// Order the Queue
        Q_SCRIPTABLE void orderQueue();

        /// Reindex the queue
        Q_SCRIPTABLE void reindexQueue();

        /// Get all the custom groups
        Q_SCRIPTABLE QStringList groups() const;

        /// Add a group
        Q_SCRIPTABLE bool addGroup(const QString& group);

        /// Remove a group
        Q_SCRIPTABLE bool removeGroup(const QString& group);

        /// Get a torrent (this is for scripting)
        Q_SCRIPTABLE QObject* torrent(const QString& info_hash);

        /// Get a group (this is for scripting)
        Q_SCRIPTABLE QObject* group(const QString& name);

        /// Get the settings (this is for scripting)
        Q_SCRIPTABLE QObject* settings();

        /// Write something to the log
        Q_SCRIPTABLE void log(const QString& line);

        ///  Get the number of torrents running.
        Q_SCRIPTABLE uint numTorrentsRunning() const;

        ///  Get the number of torrents not running.
        Q_SCRIPTABLE uint numTorrentsNotRunning() const;

        ///  Get the number of torrents not running.
        Q_SCRIPTABLE QString dataDir() const;

    private Q_SLOTS:
        void torrentAdded(bt::TorrentInterface* tc);
        void torrentRemoved(bt::TorrentInterface* tc);
        void finished(bt::TorrentInterface* tc);
        void torrentStoppedByError(bt::TorrentInterface* tc, QString msg);
        void groupAdded(Group* g);
        void groupRemoved(Group* g);
        void delayedTorrentRemoval();

    Q_SIGNALS:
        /// DBus signal emitted when a torrent has been added
        Q_SCRIPTABLE void torrentAdded(const QString& tor);

        /// DBus signal emitted when a torrent has been removed
        Q_SCRIPTABLE void torrentRemoved(const QString& tor);

        /// Emitted when torrent is finished
        Q_SCRIPTABLE void finished(const QString& tor);

        /// Emitted when a torrent is stopped by an error
        Q_SCRIPTABLE void torrentStoppedByError(const QString& tor, const QString& msg);

        /// Emitted when settings are changed in settings dialog
        Q_SCRIPTABLE void settingsChanged();

        /// Emitted when suspended state changes
        Q_SCRIPTABLE void suspendStateChanged(bool suspended);


    private:
        GUIInterface* gui;
        CoreInterface* core;
        bt::PtrMap<QString, DBusTorrent> torrent_map;
        bt::PtrMap<Group*, DBusGroup> group_map;
        QMap<QString, bool> delayed_removal_map;
        DBusSettings* dbus_settings;

        typedef bt::PtrMap<QString, DBusTorrent>::iterator DBusTorrentItr;
        typedef bt::PtrMap<Group*, DBusGroup>::iterator DBusGroupItr;
    };

}

#endif
