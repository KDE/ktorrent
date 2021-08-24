/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "upnpplugin.h"

#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/guiinterface.h>
#include <upnp/upnpmcastsocket.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

#include "upnpwidget.h"
#include <interfaces/torrentactivityinterface.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_upnp, "ktorrent_upnp.json", registerPlugin<kt::UPnPPlugin>();)

using namespace bt;

namespace kt
{
UPnPPlugin::UPnPPlugin(QObject *parent, const QVariantList & /*args*/)
    : Plugin(parent)
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
    GUIInterface *gui = getGUI();
    gui->getTorrentActivity()->addToolWidget(upnp_tab, i18n("UPnP"), QStringLiteral("kt-upnp"), i18n("Shows the status of the UPnP plugin"));
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

void UPnPPlugin::shutdown(bt::WaitJob *job)
{
    upnp_tab->shutdown(job);
}

}
#include "upnpplugin.moc"
