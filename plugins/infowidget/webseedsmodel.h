/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTWEBSEEDSMODEL_H
#define KTWEBSEEDSMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include <interfaces/torrentinterface.h>
#include <util/constants.h>

namespace kt
{
/**
    @author
*/
class WebSeedsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    WebSeedsModel(QObject *parent);
    ~WebSeedsModel() override;

    /**
     * Change the current torrent.
     * @param tc
     */
    void changeTC(bt::TorrentInterface *tc);

    /**
     *  See if we need to update the model
     */
    bool update();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    struct Item {
        QString status;
        bt::Uint64 downloaded;
        bt::Uint32 speed;
    };
    bt::TorrentInterface::WPtr curr_tc;
    QVector<Item> items;
};

}

#endif
