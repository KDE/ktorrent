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

#include <KActionCollection>
#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginFactory>

#include "downloadorderplugin.h"
#include <util/fileops.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include "downloadordermanager.h"
#include "downloadorderdialog.h"


K_PLUGIN_FACTORY_WITH_JSON(ktorrent_downloadorder, "ktorrent_downloadorder.json", registerPlugin<kt::DownloadOrderPlugin>();)

using namespace bt;

namespace kt
{
    DownloadOrderPlugin::DownloadOrderPlugin(QObject* parent, const QVariantList &args): Plugin(parent)
    {
        Q_UNUSED(args);
        download_order_action = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), i18n("File Download Order"), this);
        connect(download_order_action, &QAction::triggered, this, &DownloadOrderPlugin::showDownloadOrderDialog);
        actionCollection()->addAction(QStringLiteral("download_order"), download_order_action);
        setXMLFile(QStringLiteral("ktorrent_downloadorderui.rc"));
        managers.setAutoDelete(true);
    }


    DownloadOrderPlugin::~DownloadOrderPlugin()
    {
    }


    bool DownloadOrderPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(VERSION);
    }

    void DownloadOrderPlugin::load()
    {
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->addViewListener(this);
        connect(getCore(), &CoreInterface::torrentAdded, this, &DownloadOrderPlugin::torrentAdded);
        connect(getCore(), &CoreInterface::torrentRemoved, this, &DownloadOrderPlugin::torrentRemoved);
        currentTorrentChanged(ta->getCurrentTorrent());

        const kt::QueueManager* const qman = getCore()->getQueueManager();
        for (bt::TorrentInterface* i : *qman)
            torrentAdded(i);
    }

    void DownloadOrderPlugin::unload()
    {
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->removeViewListener(this);
        disconnect(getCore(), &CoreInterface::torrentAdded, this, &DownloadOrderPlugin::torrentAdded);
        disconnect(getCore(), &CoreInterface::torrentRemoved, this, &DownloadOrderPlugin::torrentRemoved);
        managers.clear();
    }

    void DownloadOrderPlugin::showDownloadOrderDialog()
    {
        bt::TorrentInterface* tor = getGUI()->getTorrentActivity()->getCurrentTorrent();
        if (!tor || !tor->getStats().multi_file_torrent)
            return;

        DownloadOrderDialog dlg(this, tor, getGUI()->getMainWindow());
        dlg.exec();
    }

    void DownloadOrderPlugin::currentTorrentChanged(bt::TorrentInterface* tc)
    {
        download_order_action->setEnabled(tc && tc->getStats().multi_file_torrent);
    }

    DownloadOrderManager* DownloadOrderPlugin::manager(bt::TorrentInterface* tc)
    {
        return managers.find(tc);
    }

    DownloadOrderManager* DownloadOrderPlugin::createManager(bt::TorrentInterface* tc)
    {
        DownloadOrderManager* m = manager(tc);
        if (m)
            return m;

        m = new DownloadOrderManager(tc);
        managers.insert(tc, m);
        return m;
    }

    void DownloadOrderPlugin::destroyManager(bt::TorrentInterface* tc)
    {
        managers.erase(tc);
    }

    void DownloadOrderPlugin::torrentAdded(bt::TorrentInterface* tc)
    {
        if (bt::Exists(tc->getTorDir() + QStringLiteral("download_order")))
        {
            DownloadOrderManager* m = createManager(tc);
            m->load();
            m->update();
            connect(tc, &bt::TorrentInterface::chunkDownloaded, m, &DownloadOrderManager::chunkDownloaded);
        }
    }

    void DownloadOrderPlugin::torrentRemoved(bt::TorrentInterface* tc)
    {
        managers.erase(tc);
    }
}

#include <downloadorderplugin.moc>
