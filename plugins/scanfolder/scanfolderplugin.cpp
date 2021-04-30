/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

#include <QDir>

#include <KLocalizedString>
#include <KPluginFactory>

#include "scanfolder.h"
#include "scanfolderplugin.h"
#include "scanfolderpluginsettings.h"
#include "scanfolderprefpage.h"
#include "scanthread.h"
#include "torrentloadqueue.h"

using namespace bt;

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_scanfolder, "ktorrent_scanfolder.json", registerPlugin<kt::ScanFolderPlugin>();)

namespace kt
{
ScanFolderPlugin::ScanFolderPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , tlq(nullptr)
{
    Q_UNUSED(args);
}

ScanFolderPlugin::~ScanFolderPlugin()
{
}

void ScanFolderPlugin::load()
{
    LogSystemManager::instance().registerSystem(i18nc("plugin name", "Scan Folder"), SYS_SNF);
    tlq = new TorrentLoadQueue(getCore(), this);
    scanner = new ScanThread();
    connect(scanner, &ScanThread::found, tlq, qOverload<const QList<QUrl> &>(&TorrentLoadQueue::add), Qt::QueuedConnection);
    pref = new ScanFolderPrefPage(this, nullptr);
    getGUI()->addPrefPage(pref);
    connect(getCore(), &CoreInterface::settingsChanged, this, &ScanFolderPlugin::updateScanFolders);
    scanner->start(QThread::IdlePriority);
    updateScanFolders();
}

void ScanFolderPlugin::unload()
{
    LogSystemManager::instance().unregisterSystem(i18nc("plugin name", "Scan Folder"));
    disconnect(getCore(), &CoreInterface::settingsChanged, this, &ScanFolderPlugin::updateScanFolders);
    getGUI()->removePrefPage(pref);
    scanner->stop();
    delete scanner;
    scanner = nullptr;
    delete pref;
    pref = nullptr;
    delete tlq;
    tlq = nullptr;
}

void ScanFolderPlugin::updateScanFolders()
{
    QStringList folders = ScanFolderPluginSettings::folders();

    // make sure folders end with /
    for (QString &s : folders) {
        if (s.endsWith(bt::DirSeparator()))
            s += bt::DirSeparator();
    }

    if (ScanFolderPluginSettings::actionDelete())
        tlq->setLoadedTorrentAction(DeleteAction);
    else if (ScanFolderPluginSettings::actionMove())
        tlq->setLoadedTorrentAction(MoveAction);
    else
        tlq->setLoadedTorrentAction(DefaultAction);

    scanner->setRecursive(ScanFolderPluginSettings::recursive());
    scanner->setFolderList(folders);
}

bool ScanFolderPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}
}

#include "scanfolderplugin.moc"
