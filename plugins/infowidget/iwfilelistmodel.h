/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTIWFILELISTMODEL_H
#define KTIWFILELISTMODEL_H

#include <torrent/torrentfilelistmodel.h>

namespace kt
{
/**
 *
 * @author Joris Guisson
 *
 * Expands the standard TorrentFileListModel to show more information.
 */
class IWFileListModel : public TorrentFileListModel
{
    Q_OBJECT
public:
    IWFileListModel(bt::TorrentInterface *tc, QObject *parent);
    ~IWFileListModel() override;

    void changeTorrent(bt::TorrentInterface *tc) override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void update() override;

    void filePercentageChanged(bt::TorrentFileInterface *file, float percentage) override;
    void filePreviewChanged(bt::TorrentFileInterface *file, bool preview) override;

private:
    QVariant displayData(const QModelIndex &index) const;
    QVariant sortData(const QModelIndex &index) const;

private:
    bool preview;
    bool mmfile;
    double percentage;
};

}

#endif
