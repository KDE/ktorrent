/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTTRACKERMODEL_H
#define KTTRACKERMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QUrl>

#include <interfaces/trackerinterface.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
    @author
*/
class TrackerModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TrackerModel(QObject *parent);
    ~TrackerModel() override;

    void changeTC(bt::TorrentInterface *tc);
    void update();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /// Get a tracker url given a model index
    QUrl trackerUrl(const QModelIndex &idx);

    /// Get a tracker given a model index
    bt::TrackerInterface *tracker(const QModelIndex &idx);

    /// Add trackers to the model
    void addTrackers(QList<bt::TrackerInterface *> &tracker_list);

private:
    struct Item {
        bt::TrackerInterface *trk;
        bt::TrackerStatus status;
        int seeders;
        int leechers;
        int times_downloaded;
        unsigned int time_to_next_update;

        Item(bt::TrackerInterface *tracker);
        bool update();
        QVariant displayData(int column) const;
        QVariant sortData(int column) const;
    };

    bt::TorrentInterface *tc;
    QList<Item *> trackers;
    bool running;
};

}

#endif
