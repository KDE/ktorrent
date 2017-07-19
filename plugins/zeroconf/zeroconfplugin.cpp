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
#include "zeroconfplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <util/log.h>
#include <util/logsystemmanager.h>
#include <torrent/queuemanager.h>
#include "torrentservice.h"

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_zeroconf, "ktorrent_zeroconf.json", registerPlugin<kt::ZeroConfPlugin>();)

using namespace bt;

namespace kt
{

    ZeroConfPlugin::ZeroConfPlugin(QObject* parent, const QVariantList& args) : Plugin(parent)
    {
        Q_UNUSED(args);
        services.setAutoDelete(true);
    }


    ZeroConfPlugin::~ZeroConfPlugin()
    {}

    void ZeroConfPlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18n("ZeroConf"), SYS_ZCO);
        CoreInterface* core = getCore();
        connect(core, &CoreInterface::torrentAdded, this, &ZeroConfPlugin::torrentAdded);
        connect(core, &CoreInterface::torrentRemoved, this, &ZeroConfPlugin::torrentRemoved);

        // go over existing torrents and add them
        kt::QueueManager* qman = core->getQueueManager();
        for (QList<bt::TorrentInterface*>::iterator i = qman->begin(); i != qman->end(); i++)
        {
            torrentAdded(*i);
        }
    }

    void ZeroConfPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18n("ZeroConf"));
        CoreInterface* core = getCore();
        disconnect(core, SIGNAL(torrentAdded(bt::TorrentInterface*)),
                   this, SLOT(torrentAdded(bt::TorrentInterface*)));
        disconnect(core, SIGNAL(torrentRemoved(bt::TorrentInterface*)),
                   this, SLOT(torrentRemoved(bt::TorrentInterface*)));

        bt::PtrMap<bt::TorrentInterface*, TorrentService>::iterator i = services.begin();
        while (i != services.end())
        {
            TorrentService* av = i->second;
            bt::TorrentInterface* ti = i->first;
            ti->removePeerSource(av);
            i++;
        }
        services.clear();
    }

    void ZeroConfPlugin::torrentAdded(bt::TorrentInterface* tc)
    {
        if (services.contains(tc))
            return;


        TorrentService* av = new TorrentService(tc);
        services.insert(tc, av);
        tc->addPeerSource(av);
        Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service added for " << tc->getStats().torrent_name << endl;
        connect(av, &TorrentService::serviceDestroyed, this, &ZeroConfPlugin::avahiServiceDestroyed);
    }


    void ZeroConfPlugin::torrentRemoved(bt::TorrentInterface* tc)
    {
        TorrentService* av = services.find(tc);
        if (!av)
            return;
        Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service removed for "
                                  << tc->getStats().torrent_name << endl;
        tc->removePeerSource(av);
        services.erase(tc);
    }

    void ZeroConfPlugin::avahiServiceDestroyed(TorrentService* av)
    {
        services.setAutoDelete(false);

        Out(SYS_ZCO | LOG_NOTICE) << "ZeroConf service destroyed " << endl;
        bt::PtrMap<bt::TorrentInterface*, TorrentService>::iterator i = services.begin();
        while (i != services.end())
        {
            if (i->second == av)
            {
                services.erase(i->first);
                break;
            }
            i++;
        }
        services.setAutoDelete(true);
    }

    bool ZeroConfPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }
}
#include "zeroconfplugin.moc"
