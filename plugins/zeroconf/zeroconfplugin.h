/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
 *   joris.guisson@gmail.com                                               *
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

#ifndef KTZEROCONFPLUGIN_H
#define KTZEROCONFPLUGIN_H

#include <util/ptrmap.h>
#include <interfaces/plugin.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
    class TorrentService;

    /**
     * @author Joris Guisson <joris.guisson@gmail.com>
     *
     * Plugin which handles the zeroconf service.
     */
    class ZeroConfPlugin : public Plugin
    {
        Q_OBJECT
    public:
        ZeroConfPlugin(QObject* parent, const QVariantList& args);
        ~ZeroConfPlugin();

        void load() override;
        void unload() override;
        bool versionCheck(const QString& version) const override;

    private slots:
        /**
         * A TorrentInterface was added
         * @param tc
         */
        void torrentAdded(bt::TorrentInterface* tc);

        /**
         * A TorrentInterface was removed
         * @param tc
         */
        void torrentRemoved(bt::TorrentInterface* tc);

        /**
         * An AvahiService has been destroyed by the psman
         */
        void avahiServiceDestroyed(TorrentService* av);

    private:
        bt::PtrMap<bt::TorrentInterface*, TorrentService> services;
    };

}

#endif
