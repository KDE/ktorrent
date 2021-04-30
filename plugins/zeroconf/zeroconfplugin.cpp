/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "zeroconfplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include "torrentservice.h"
#include <torrent/queuemanager.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_zeroconf, "ktorrent_zeroconf.json", registerPlugin<kt::ZeroConfPlugin>();)

using namespace bt;

namespace kt
{
ZeroConfPlugin::ZeroConfPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
{
    Q_UNUSED(args);
    services.setAutoDelete(true);
}

ZeroConfPlugin::~ZeroConfPlugin()
{
}

void ZeroConfPlugin::load()
{
    LogSystemManager::instance().registerSystem(i18n("ZeroConf"), SYS_ZCO);
    CoreInterface *core = getCore();
    connect(core, &CoreInterface::torrentAdded, this, &ZeroConfPlugin::torrentAdded);
    connect(core, &CoreInterface::torrentRemoved, this, &ZeroConfPlugin::torrentRemoved);

    // go over existing torrents and add them
    const kt::QueueManager *const qman = core->getQueueManager();
    for (bt::TorrentInterface *tor : *qman) {
        torrentAdded(tor);
    }
}

void ZeroConfPlugin::unload()
{
    LogSystemManager::instance().unregisterSystem(i18n("ZeroConf"));
    CoreInterface *core = getCore();
    disconnect(core, &CoreInterface::torrentAdded, this, &ZeroConfPlugin::torrentAdded);
    disconnect(core, &CoreInterface::torrentRemoved, this, &ZeroConfPlugin::torrentRemoved);

    bt::PtrMap<bt::TorrentInterface *, TorrentService>::iterator i = services.begin();
    while (i != services.end()) {
        TorrentService *av = i->second;
        bt::TorrentInterface *ti = i->first;
        ti->removePeerSource(av);
        i++;
    }
    services.clear();
}

void ZeroConfPlugin::torrentAdded(bt::TorrentInterface *tc)
{
    if (services.contains(tc))
        return;

    TorrentService *av = new TorrentService(tc);
    services.insert(tc, av);
    tc->addPeerSource(av);
    Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service added for " << tc->getStats().torrent_name << endl;
    connect(av, &TorrentService::serviceDestroyed, this, &ZeroConfPlugin::avahiServiceDestroyed);
}

void ZeroConfPlugin::torrentRemoved(bt::TorrentInterface *tc)
{
    TorrentService *av = services.find(tc);
    if (!av)
        return;
    Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service removed for " << tc->getStats().torrent_name << endl;
    tc->removePeerSource(av);
    services.erase(tc);
}

void ZeroConfPlugin::avahiServiceDestroyed(TorrentService *av)
{
    services.setAutoDelete(false);

    Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service destroyed " << endl;
    bt::PtrMap<bt::TorrentInterface *, TorrentService>::iterator i = services.begin();
    while (i != services.end()) {
        if (i->second == av) {
            services.erase(i->first);
            break;
        }
        i++;
    }
    services.setAutoDelete(true);
}

bool ZeroConfPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}
}
#include "zeroconfplugin.moc"
