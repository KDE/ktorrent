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

#include "upnpplugin.h"

#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/guiinterface.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include <upnp/upnpmcastsocket.h>

#include "upnpwidget.h"
#include <interfaces/torrentactivityinterface.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_upnp, "ktorrent_upnp.json", registerPlugin<kt::UPnPPlugin>();)

using namespace bt;

namespace kt
{

    UPnPPlugin::UPnPPlugin(QObject* parent, const QVariantList& /*args*/) : Plugin(parent), sock(nullptr), upnp_tab(nullptr)
    {
    }


    UPnPPlugin::~UPnPPlugin()
    {
    }


    void UPnPPlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18n("UPnP"), SYS_PNP);
        sock = new UPnPMCastSocket();
        upnp_tab = new UPnPWidget(sock, nullptr);
        GUIInterface* gui = getGUI();
        gui->getTorrentActivity()->addToolWidget(upnp_tab, i18n("UPnP"), QStringLiteral("kt-upnp"),
                i18n("Shows the status of the UPnP plugin"));
        // load the routers list
        QString routers_file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("routers"));
        if (routers_file.length())
            sock->loadRouters(routers_file);
        sock->discover();
    }

    void UPnPPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18n("UPnP"));
        QString routers_file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/routers");
        sock->saveRouters(routers_file);
        getGUI()->getTorrentActivity()->removeToolWidget(upnp_tab);
        sock->close();
        delete upnp_tab;
        upnp_tab = nullptr;
        delete sock;
        sock = nullptr;
    }

    void UPnPPlugin::shutdown(bt::WaitJob* job)
    {
        upnp_tab->shutdown(job);
    }

    bool UPnPPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }
}
#include "upnpplugin.moc"
