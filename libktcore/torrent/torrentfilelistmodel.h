/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTTORRENTFILELISTMODEL_H
#define KTTORRENTFILELISTMODEL_H

#include "torrentfilemodel.h"

namespace kt
{
/**
 * Model for displaying file trees of a torrent
 * @author Joris Guisson
 */
class KTCORE_EXPORT TorrentFileListModel : public TorrentFileModel
{
    Q_OBJECT
public:
    TorrentFileListModel(bt::TorrentInterface *tc, DeselectMode mode, QObject *parent);
    ~TorrentFileListModel() override;

    void changeTorrent(bt::TorrentInterface *tc) override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void checkAll() override;
    void uncheckAll() override;
    void invertCheck() override;
    bt::Uint64 bytesToDownload() override;
    bt::TorrentFileInterface *indexToFile(const QModelIndex &idx) override;
    QString dirPath(const QModelIndex &idx) override;
    void changePriority(const QModelIndexList &indexes, bt::Priority newpriority) override;

private:
    void invertCheck(const QModelIndex &idx);
};

}

#endif
