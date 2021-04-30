/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scanthread.h"

#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QTimer>

#include <KLocalizedString>

#include <util/fileops.h>

namespace kt
{
const int UPDATE_FOLDER_EVENT = QEvent::User + 1;
const int RECURSIVE_SCAN_EVENT = QEvent::User + 2;

class UpdateFolderEvent : public QEvent
{
public:
    UpdateFolderEvent()
        : QEvent((QEvent::Type)UPDATE_FOLDER_EVENT)
    {
    }

    ~UpdateFolderEvent() override
    {
    }
};

class RecursiveScanEvent : public QEvent
{
public:
    RecursiveScanEvent(const QUrl &url)
        : QEvent((QEvent::Type)RECURSIVE_SCAN_EVENT)
        , url(url)
    {
    }

    ~RecursiveScanEvent() override
    {
    }

    QUrl url;
};

ScanThread::ScanThread()
    : stop_requested(false)
    , recursive(false)
{
    scan_folders.setAutoDelete(true);
    moveToThread(this);
}

ScanThread::~ScanThread()
{
}

void ScanThread::setRecursive(bool rec)
{
    recursive = rec;
}

void ScanThread::addDirectory(const QUrl &url, bool recursive)
{
    scan(url, recursive);
}

void ScanThread::setFolderList(const QStringList &folders)
{
    QMutexLocker lock(&mutex);
    if (this->folders != folders) {
        this->folders = folders;
        // Use custom event to wake up scanner thread
        QCoreApplication::postEvent(this, new UpdateFolderEvent());
    }
}

void ScanThread::customEvent(QEvent *ev)
{
    if (ev->type() == UPDATE_FOLDER_EVENT) {
        updateFolders();
    } else if (ev->type() == RECURSIVE_SCAN_EVENT) {
        RecursiveScanEvent *rev = (RecursiveScanEvent *)ev;
        scan(rev->url, true);
    }
    ev->accept();
}

void ScanThread::updateFolders()
{
    QStringList tmp;

    mutex.lock();
    tmp = folders; // Use tmp list to not block the mutex for to long
    mutex.unlock();

    // first erase folders we don't need anymore
    bt::PtrMap<QString, ScanFolder>::iterator i = scan_folders.begin();
    while (i != scan_folders.end()) {
        if (!tmp.contains(i->first)) {
            QString f = i->first;
            i++;
            scan_folders.erase(f);
        } else {
            i->second->setRecursive(recursive);
            i++;
        }
    }

    for (const QString &folder : qAsConst(tmp)) {
        if (scan_folders.find(folder))
            continue;

        if (QDir(folder).exists()) {
            // only add folder when it exists
            ScanFolder *sf = new ScanFolder(this, QUrl::fromLocalFile(folder), recursive);
            scan_folders.insert(folder, sf);
        }
    }
}

void ScanThread::run()
{
    updateFolders();
    exec();
}

void ScanThread::stop()
{
    stop_requested = true;

    // XXX seems like deleting KDirWatch object(s) created in scan_folders
    // in destructor of this QThread after it has been stopped
    // causes memory corruption, so we delete them early
    scan_folders.clear();
    exit();
    wait();
}

bool ScanThread::alreadyLoaded(const QDir &d, const QString &torrent)
{
    return d.exists(QLatin1Char('.') + torrent);
}

void ScanThread::scan(const QUrl &dir, bool recursive)
{
    if (stop_requested)
        return;

    QStringList filters;
    filters << QStringLiteral("*.torrent");
    QDir d(dir.toLocalFile());
    const QStringList files = d.entryList(filters, QDir::Readable | QDir::Files);

    QList<QUrl> torrents;
    for (const QString &tor : files) {
        if (!alreadyLoaded(d, tor))
            torrents.append(QUrl::fromLocalFile(d.absoluteFilePath(tor)));
    }

    found(torrents);

    if (stop_requested)
        return;

    if (recursive) {
        const QString loaded_localized = i18nc("folder name part", "loaded");

        const QStringList dirs = d.entryList(QDir::Readable | QDir::Dirs);
        for (const QString &subdir : dirs) {
            if (subdir != QStringLiteral(".") && subdir != QStringLiteral("..") && subdir != loaded_localized) {
                QCoreApplication::postEvent(this, new RecursiveScanEvent(QUrl::fromLocalFile(d.absoluteFilePath(subdir))));
            }
        }
    }
}

}
