/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef KTIPFILTERPLUGIN_H
#define KTIPFILTERPLUGIN_H

#include <QTimer>
#include <interfaces/plugin.h>
#include "ipblockingprefpage.h"
#include "ipblocklist.h"

class QString;

namespace kt
{
    class IPBlockingPrefPage;

    const int AUTO_UPDATE_RETRY_INTERVAL = 15 * 60; // seconds

    /**
     * @author Ivan Vasic <ivasic@gmail.com>
     * @brief IP filter plugin
     *
     * This plugin will load IP ranges from specific files into KT IPBlocklist.
     */
    class IPFilterPlugin : public Plugin
    {
        Q_OBJECT
    public:
        IPFilterPlugin(QObject* parent, const QVariantList& args);
        ~IPFilterPlugin();

        void load() override;
        void unload() override;
        bool versionCheck(const QString& version) const override;

        ///Loads the KT format list filter
        void loadFilters();

        ///Loads the anti-p2p filter list
        bool loadAntiP2P();

        ///Unloads the anti-p2p filter list
        bool unloadAntiP2P();

        /// Whether or not the IP filter is loaded and running
        bool loadedAndRunning();

    public slots:
        void checkAutoUpdate();
        void notification(const QString& msg);

    private:
        IPBlockingPrefPage* pref;
        QScopedPointer<IPBlockList> ip_filter;
        QTimer auto_update_timer;
    };

}

#endif
