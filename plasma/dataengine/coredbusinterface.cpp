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

#include "coredbusinterface.h"
#include "engine.h"

namespace ktplasma
{

    CoreDBusInterface::CoreDBusInterface(Engine* engine)
        : QObject(engine), engine(engine)
    {
        QDBusConnection con = QDBusConnection::sessionBus();
        core = new QDBusInterface("org.ktorrent.ktorrent", "/core", "org.ktorrent.core", con, this);
        engine->setData("core", "connected", true);
        engine->setData("core", "num_torrents", 0);

        con.connect("org.ktorrent.ktorrent", "/core", "org.ktorrent.core", "torrentAdded", this, SLOT(torrentAdded(const QString&)));
        con.connect("org.ktorrent.ktorrent", "/core", "org.ktorrent.core", "torrentRemoved", this, SLOT(torrentRemoved(const QString&)));
    }


    CoreDBusInterface::~CoreDBusInterface()
    {
    }

    void CoreDBusInterface::init()
    {
        QDBusReply<QStringList> r = core->call("torrents");
        if (r.isValid())
        {
            QStringList torrents = r.value();
            engine->setData("core", "num_torrents", torrents.count());
            foreach (const QString& tor, torrents)
            {
                engine->addTorrent(tor);
            }
        }
    }

    void CoreDBusInterface::update()
    {
    }

    void CoreDBusInterface::torrentAdded(const QString& tor)
    {
        engine->addTorrent(tor);
    }

    void CoreDBusInterface::torrentRemoved(const QString& tor)
    {
        engine->removeTorrent(tor);
    }
}
