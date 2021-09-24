/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KActionCollection>
#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginFactory>

#include "downloadorderdialog.h"
#include "downloadordermanager.h"
#include "downloadorderplugin.h"
#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <util/fileops.h>

K_PLUGIN_CLASS_WITH_JSON(kt::DownloadOrderPlugin, "ktorrent_downloadorder.json")

using namespace bt;

namespace kt
{
DownloadOrderPlugin::DownloadOrderPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plugin(parent, data, args)
{
    download_order_action = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), i18n("File Download Order"), this);
    connect(download_order_action, &QAction::triggered, this, &DownloadOrderPlugin::showDownloadOrderDialog);
    actionCollection()->addAction(QStringLiteral("download_order"), download_order_action);
    setXMLFile(QStringLiteral("ktorrent_downloadorderui.rc"));
    managers.setAutoDelete(true);
}

DownloadOrderPlugin::~DownloadOrderPlugin()
{
}

void DownloadOrderPlugin::load()
{
    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->addViewListener(this);
    connect(getCore(), &CoreInterface::torrentAdded, this, &DownloadOrderPlugin::torrentAdded);
    connect(getCore(), &CoreInterface::torrentRemoved, this, &DownloadOrderPlugin::torrentRemoved);
    currentTorrentChanged(ta->getCurrentTorrent());

    const kt::QueueManager *const qman = getCore()->getQueueManager();
    for (bt::TorrentInterface *i : *qman)
        torrentAdded(i);
}

void DownloadOrderPlugin::unload()
{
    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->removeViewListener(this);
    disconnect(getCore(), &CoreInterface::torrentAdded, this, &DownloadOrderPlugin::torrentAdded);
    disconnect(getCore(), &CoreInterface::torrentRemoved, this, &DownloadOrderPlugin::torrentRemoved);
    managers.clear();
}

void DownloadOrderPlugin::showDownloadOrderDialog()
{
    bt::TorrentInterface *tor = getGUI()->getTorrentActivity()->getCurrentTorrent();
    if (!tor || !tor->getStats().multi_file_torrent)
        return;

    DownloadOrderDialog dlg(this, tor, getGUI()->getMainWindow());
    dlg.exec();
}

void DownloadOrderPlugin::currentTorrentChanged(bt::TorrentInterface *tc)
{
    download_order_action->setEnabled(tc && tc->getStats().multi_file_torrent);
}

DownloadOrderManager *DownloadOrderPlugin::manager(bt::TorrentInterface *tc)
{
    return managers.find(tc);
}

DownloadOrderManager *DownloadOrderPlugin::createManager(bt::TorrentInterface *tc)
{
    DownloadOrderManager *m = manager(tc);
    if (m)
        return m;

    m = new DownloadOrderManager(tc);
    managers.insert(tc, m);
    return m;
}

void DownloadOrderPlugin::destroyManager(bt::TorrentInterface *tc)
{
    managers.erase(tc);
}

void DownloadOrderPlugin::torrentAdded(bt::TorrentInterface *tc)
{
    if (bt::Exists(tc->getTorDir() + QStringLiteral("download_order"))) {
        DownloadOrderManager *m = createManager(tc);
        m->load();
        m->update();
        connect(tc, &bt::TorrentInterface::chunkDownloaded, m, &DownloadOrderManager::chunkDownloaded);
    }
}

void DownloadOrderPlugin::torrentRemoved(bt::TorrentInterface *tc)
{
    managers.erase(tc);
}
}

#include <downloadorderplugin.moc>
