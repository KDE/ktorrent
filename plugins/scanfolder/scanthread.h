/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_SCANTHREAD_H
#define KT_SCANTHREAD_H

#include <QMutex>
#include <QStringList>
#include <QThread>
#include <QUrl>

#include "scanfolder.h"
#include <util/ptrmap.h>

#include <atomic>

class QDir;

namespace kt
{
/**
 * Thread which scans directories in the background and looks for torrent files.
 */
class ScanThread : public QThread
{
    Q_OBJECT
public:
    ScanThread();
    ~ScanThread() override;

    /**
     * Set whether to scan recursively or not
     * @param rec Recursive or not
     */
    void setRecursive(bool rec);

    /**
     * Add a directory to scan.
     * @param url Directory
     * @param recursive Whether or not to scan resursively
     */
    void addDirectory(const QUrl &url, bool recursive);

    /**
     * Stop the scanning thread.
     */
    void stop();

    /**
     * Set the list of folders to scan.
     * @param folders List of folders
     */
    void setFolderList(const QStringList &folders);

protected:
    void run() override;

private:
    void scan(const QUrl &dir, bool recursive);
    bool alreadyLoaded(const QDir &d, const QString &torrent);
    void updateFolders();
    void customEvent(QEvent *ev) override;

Q_SIGNALS:
    /**
     * Emitted when one or more torrents are found.
     * @param torrents The list of torrents
     */
    void found(const QList<QUrl> &torrents);

private:
    QMutex mutex;
    QStringList folders;
    std::atomic<bool> stop_requested;
    std::atomic<bool> recursive;
    bt::PtrMap<QString, ScanFolder> scan_folders;
};

}

#endif // KT_SCANTHREAD_H
