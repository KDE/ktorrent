/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTCHUNKDOWNLOADMODEL_H
#define KTCHUNKDOWNLOADMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
/**
    @author
*/
class ChunkDownloadModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ChunkDownloadModel(QObject *parent);
    ~ChunkDownloadModel() override;

    /// A peer has been added
    void downloadAdded(bt::ChunkDownloadInterface *cd);

    /// A download has been removed
    void downloadRemoved(bt::ChunkDownloadInterface *cd);

    /// change the current torrent
    void changeTC(bt::TorrentInterface *tc);

    /**
     * Update the model
     */
    void update();

    /**
        Clear the model
    */
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

public:
    struct Item {
        mutable bt::ChunkDownloadInterface::Stats stats;
        bt::ChunkDownloadInterface *cd;
        QString files;

        Item(bt::ChunkDownloadInterface *cd, const QString &files);

        bool changed() const;
        QVariant data(int col) const;
        QVariant sortData(int col) const;
    };

private:
    QVector<Item *> items;
    bt::TorrentInterface::WPtr tc;
};

}

#endif
