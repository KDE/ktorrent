/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scanforlostfilesthread.h"

#include "nodeoperations.h"
#include <interfaces/coreinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>

namespace kt
{
ScanForLostFilesThread::ScanForLostFilesThread(const QString &folder, CoreInterface *core, QObject *parent)
    : QThread(parent)
    , m_core(core)
{
    m_root_folder = folder;
    while (m_root_folder.endsWith(QLatin1String("/")) && m_root_folder != QLatin1String("/")) {
        m_root_folder.chop(1);
    }
}

void ScanForLostFilesThread::run()
{
    if (!m_core) {
        Q_EMIT filterReady(nullptr);
        return;
    }

    FNode *torrent_files = new FNode();
    FNode *torrent_folders = new FNode();
    NodeOperations::makePath(torrent_files, m_root_folder, true);

    kt::QueueManager *qman = m_core->getQueueManager();
    if (qman) {
        QList<bt::TorrentInterface *>::iterator it = qman->begin();
        while (it != qman->end()) {
            if (isInterruptionRequested())
                break;

            bt::TorrentInterface *tor = *it;
            if (tor->getStats().multi_file_torrent) {
                for (bt::Uint32 i = 0; i < tor->getNumFiles(); i++) {
                    NodeOperations::makePath(torrent_files, tor->getTorrentFile(i).getPathOnDisk(), false);
                }
                if (tor->getNumFiles()) {
                    QString folderpath = tor->getTorrentFile(0).getPathOnDisk();
                    int idx = folderpath.lastIndexOf(tor->getTorrentFile(0).getUserModifiedPath());
                    QString out_folder = folderpath.left(idx - 1);
                    NodeOperations::makePath(torrent_folders, out_folder, true);
                }
            } else
                NodeOperations::makePath(torrent_files, tor->getStats().output_path, false);

            it++;
        }
    }

    FNode *existing_files = new FNode();
    FNode *folder_node = NodeOperations::makePath(existing_files, m_root_folder, true);
    QDir dir(m_root_folder);

    if (!isInterruptionRequested()) {
        NodeOperations::fillFromDir(folder_node, dir);
        NodeOperations::subtractTreesOnFiles(existing_files, torrent_files);
        NodeOperations::pruneEmptyFolders(existing_files, torrent_folders);
    }

    QSet<QString> *filter = new QSet<QString>();
    NodeOperations::printTree(existing_files, *filter);
    Q_EMIT filterReady(filter);

    NodeOperations::removeNode(torrent_files);
    NodeOperations::removeNode(torrent_folders);
    NodeOperations::removeNode(existing_files);
}

}
