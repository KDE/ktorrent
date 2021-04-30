/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCANFORLOSTFILESTHREAD_H
#define SCANFORLOSTFILESTHREAD_H

#include <QSet>
#include <QThread>

namespace kt
{
class CoreInterface;

/**
 * ScanForLostFiles working thread. It:
 * 1. Lists the folder content with QDirIterator
 * 2. Lists the files that belongs to torrents
 * 3. Substracts the results of 2nd step from results of 1st step
 * 4. Returns resulting file tree as a set of filepaths by emitting filterReady signal
 */

class ScanForLostFilesThread : public QThread
{
    Q_OBJECT
public:
    /**
     * Set the list of folders to scan.
     * @param folder A folder whose content shall be displayed
     * @param core   A core interface pointer to get list of torrent files
     */
    ScanForLostFilesThread(const QString &folder, CoreInterface *core, QObject *parent = nullptr);

protected:
    void run() override;

Q_SIGNALS:
    /**
     * Emitted when filter generation is complete.
     * @param filter Pointer to set of filepaths that are not belong to any torrent
     */
    void filterReady(QSet<QString> *filter);

private:
    QString m_root_folder;
    CoreInterface *m_core;
};

}
#endif // SCANFORLOSTFILESTHREAD_H
