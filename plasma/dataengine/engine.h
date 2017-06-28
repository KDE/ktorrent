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

#ifndef KTPLASMAENGINE_H
#define KTPLASMAENGINE_H

#include <QDBusConnectionInterface>

#include <util/ptrmap.h>
#include <plasma/dataengine.h>
#include "torrentdbusinterface.h"

namespace ktplasma
{
    class CoreDBusInterface;

    /**
        Data engine which gets all kind of information out of ktorrent
    */
    class Engine : public Plasma::DataEngine
    {
        Q_OBJECT

    public:
        Engine(QObject* parent, const QVariantList& args);
        virtual ~Engine();

        virtual bool updateSourceEvent(const QString& source);

        void addTorrent(const QString& tor);
        void removeTorrent(const QString& tor);

    public Q_SLOTS:
        void dbusServiceRegistered(const QString& name);
        void dbusServiceUnregistered(const QString& name);
        void dbusServiceOwnerChanged(const QString& name, const QString& oldOwner, const QString& newOwner);

    private:
        QDBusConnectionInterface* dbus;
        CoreDBusInterface* core;
        bt::PtrMap<QString, TorrentDBusInterface> torrent_map;

        friend class CoreDBusInterface;
        friend class TorrentDBusInterface;
    };

}


#endif
