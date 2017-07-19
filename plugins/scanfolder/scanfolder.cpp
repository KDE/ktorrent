/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                     *
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

#include "scanfolder.h"

#include <QDir>

#include <KConfigGroup>
#include <KFileItem>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KIO/Job>

#include <util/log.h>
#include <util/functions.h>
#include <util/fileops.h>
#include "torrentloadqueue.h"
#include "scanthread.h"



using namespace bt;

namespace kt
{

    ScanFolder::ScanFolder(ScanThread* scanner, const QUrl &dir, bool recursive)
        : scanner(scanner),
          scan_directory(dir),
          watch(nullptr),
          recursive(recursive)
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

    void ScanFolder::scanDir(const QString& path)
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
        if (recursive != rec)
        {
            recursive = rec;
            watch->removeDir(scan_directory.toLocalFile());
            watch->addDir(scan_directory.toLocalFile(), recursive ? KDirWatch::WatchSubDirs : KDirWatch::WatchDirOnly);
        }
    }

}
