/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scanfolder.h"

#include <QDir>

#include <KConfigGroup>
#include <KFileItem>
#include <KIO/Job>
#include <KLocalizedString>
#include <KSharedConfig>

#include "scanthread.h"
#include "torrentloadqueue.h"
#include <util/fileops.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
ScanFolder::ScanFolder(ScanThread *scanner, const QUrl &dir, bool recursive)
    : scanner(scanner)
    , scan_directory(dir)
    , watch(nullptr)
    , recursive(recursive)
{
    bt::Out(SYS_SNF | LOG_NOTICE) << "ScanFolder: scanning " << dir << endl;

    KConfigGroup config(KSharedConfig::openConfig(), "DirWatch");
    config.writeEntry("NFSPollInterval", 5000);
    config.writeEntry("nfsPreferredMethod", "Stat"); // Force the usage of Stat method for NFS
    config.sync();

    watch = new KDirWatch(this);
    connect(watch, &KDirWatch::dirty, this, &ScanFolder::scanDir);
    connect(watch, &KDirWatch::created, this, &ScanFolder::scanDir);

    watch->addDir(dir.toLocalFile(), recursive ? KDirWatch::WatchSubDirs : KDirWatch::WatchDirOnly);

    scanner->addDirectory(dir, recursive);
}

ScanFolder::~ScanFolder()
{
}

void ScanFolder::scanDir(const QString &path)
{
    if (!QFileInfo(path).isDir())
        return;

    QDir dir(path);
    if (!recursive && dir != QDir(scan_directory.toLocalFile()))
        return;

    // ignore loaded directories
    if (dir.dirName() == i18nc("folder name part", "loaded"))
        return;

    Out(SYS_SNF | LOG_NOTICE) << "Directory dirty: " << path << endl;
    scanner->addDirectory(QUrl::fromLocalFile(path), false);
}

void ScanFolder::setRecursive(bool rec)
{
    if (recursive != rec) {
        recursive = rec;
        watch->removeDir(scan_directory.toLocalFile());
        watch->addDir(scan_directory.toLocalFile(), recursive ? KDirWatch::WatchSubDirs : KDirWatch::WatchDirOnly);
    }
}

}
