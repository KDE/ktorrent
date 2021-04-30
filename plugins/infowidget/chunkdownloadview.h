/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_CHUNKDOWNLOADVIEW_HH
#define KT_CHUNKDOWNLOADVIEW_HH

#include <QSortFilterProxyModel>
#include <QTreeView>

#include <KSharedConfig>

#include "ui_chunkdownloadview.h"
#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
class ChunkDownloadModel;

/**
 * View which shows a list of downloading chunks, of a torrent.
 * */
class ChunkDownloadView : public QWidget, public Ui_ChunkDownloadView
{
    Q_OBJECT
public:
    ChunkDownloadView(QWidget *parent);
    ~ChunkDownloadView() override;

    /// A peer has been added
    void downloadAdded(bt::ChunkDownloadInterface *cd);

    /// A download has been removed
    void downloadRemoved(bt::ChunkDownloadInterface *cd);

    /// Check to see if the GUI needs to be updated
    void update();

    /// Change the torrent to display
    void changeTC(bt::TorrentInterface *tc);

    /// Remove all items
    void removeAll();

    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

private:
    bt::TorrentInterface::WPtr curr_tc;
    ChunkDownloadModel *model;
    QSortFilterProxyModel *pm;
};
}

#endif
