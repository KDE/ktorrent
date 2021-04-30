/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCANFOLDER_H
#define SCANFOLDER_H

#include <KDirWatch>
#include <QObject>
#include <QUrl>

namespace kt
{
class ScanThread;

/**
 * Monitors a folder for changes, and passes torrents to load to the TorrentLoadQueue
 */
class ScanFolder : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor.
     * @param scanner The ScanThread
     * @param dir The directory
     */
    ScanFolder(ScanThread *scanner, const QUrl &dir, bool recursive);
    ~ScanFolder() override;

    /**
     * Set if the ScanFolder needs to scan subdirectories recursively
     * @param rec Recursive or not
     */
    void setRecursive(bool rec);

public Q_SLOTS:
    void scanDir(const QString &path);

private:
    ScanThread *scanner;
    QUrl scan_directory;
    KDirWatch *watch;
    bool recursive;
};
}
#endif
