/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTIWFILETREEMODEL_H
#define KTIWFILETREEMODEL_H

#include <torrent/torrentfiletreemodel.h>

namespace kt
{
/**
 *
 * @author Joris Guisson
 *
 * Expands the standard TorrentFileTreeModel to show more information.
 */
class IWFileTreeModel : public TorrentFileTreeModel
{
    Q_OBJECT
public:
    IWFileTreeModel(bt::TorrentInterface *tc, QObject *parent);
    ~IWFileTreeModel() override;

    void changeTorrent(bt::TorrentInterface *tc) override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void update() override;
    void changePriority(const QModelIndexList &indexes, bt::Priority newpriority) override;

    void filePercentageChanged(bt::TorrentFileInterface *file, float percentage) override;
    void filePreviewChanged(bt::TorrentFileInterface *file, bool preview) override;

private:
    void update(const QModelIndex &index, bt::TorrentFileInterface *file, int col);
    QVariant displayData(Node *n, const QModelIndex &index) const;
    QVariant sortData(Node *n, const QModelIndex &index) const;
    void setPriority(Node *n, bt::Priority newpriority, bool selected_node);

private:
    bool preview;
    bool mmfile;
    double percentage;
};

}

#endif
