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

#include <QDBusInterface>
#include <KStandardDirs>

#include <util/log.h>
#include "engine.h"
#include "coredbusinterface.h"
#include "torrentdbusinterface.h"

K_EXPORT_PLASMA_DATAENGINE(ktorrent, ktplasma::Engine)

using namespace bt;

namespace ktplasma
{
    QString DataDir()
    {
        QString str = KGlobal::dirs()->saveLocation("data", "ktorrent");
        if (!str.endsWith('/'))
            return str + '/';
        else
            return str;
    }

    Engine::Engine(QObject* parent, const QVariantList& args)
        : Plasma::DataEngine(parent, args), core(0)
    {
        bt::InitLog(DataDir() + "dataengine.log", false);
        dbus = QDBusConnection::sessionBus().interface();
        connect(dbus, &QDBusConnectionInterface::serviceRegistered, this, &Engine::dbusServiceRegistered);
        connect(dbus, &QDBusConnectionInterface::serviceUnregistered, this, &Engine::dbusServiceUnregistered);
        connect(dbus, &QDBusConnectionInterface::serviceOwnerChanged, this, &Engine::dbusServiceOwnerChanged);

        torrent_map.setAutoDelete(true);
        setData("core", "connected", false);
        setData("core", "num_torrents", 0);

        if (dbus->registeredServiceNames().value().contains("org.ktorrent.ktorrent"))
            dbusServiceRegistered("org.ktorrent.ktorrent");
    }


    Engine::~Engine()
    {
    }

    bool Engine::updateSourceEvent(const QString& source)
    {
        if (torrent_map.contains(source))
        {
            torrent_map.find(source)->update();
            return true;
        }
        else if (source == "core")
        {
            core->update();
            return true;
        }

        return false;
    }

    void Engine::dbusServiceRegistered(const QString& name)
    {
        bt::Out(SYS_GEN | LOG_DEBUG) << "Engine::dbusServiceRegistered " << name << endl;
        if (name != "org.ktorrent.ktorrent")
            return;

        if (!core)
        {
            core = new CoreDBusInterface(this);
            core->init();
        }
    }

    void Engine::dbusServiceUnregistered(const QString& name)
    {
        bt::Out(SYS_GEN | LOG_DEBUG) << "Engine::dbusServiceUnregistered " << name << endl;
        if (name != "org.ktorrent.ktorrent")
            return;

        setData("core", "connected", false);
        setData("core", "num_torrents", 0);
        delete core;
        core = 0;
        for (bt::PtrMap<QString, TorrentDBusInterface>::iterator i = torrent_map.begin(); i != torrent_map.end(); i++)
        {
            removeAllData(i->first);
            removeSource(i->first);
        }
        torrent_map.clear();
    }

    void Engine::dbusServiceOwnerChanged(const QString& name, const QString& oldOwner, const QString& newOwner)
    {
        bt::Out(SYS_GEN | LOG_DEBUG) << "Engine::dbusServiceOwnerChanged " << name << " " << oldOwner << " " << newOwner << endl;
        if (name != "org.ktorrent.ktorrent")
            return;

        if (oldOwner.isEmpty() && !newOwner.isEmpty())
            dbusServiceRegistered(name);
        else if (!oldOwner.isEmpty() && newOwner.isEmpty())
            dbusServiceUnregistered(name);
        else if (!oldOwner.isEmpty() && !newOwner.isEmpty())
            dbusServiceRegistered(name);
    }

    void Engine::addTorrent(const QString& tor)
    {
        torrent_map.insert(tor, new TorrentDBusInterface(tor, this));
        updateSourceEvent(tor);
        setData("core", "num_torrents", torrent_map.count());
    }

    void Engine::removeTorrent(const QString& tor)
    {
        torrent_map.erase(tor);
        removeAllData(tor);
        removeSource(tor);
        setData("core", "num_torrents", torrent_map.count());
    }
}
