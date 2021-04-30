/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDWIDGETMODEL_H
#define KTFEEDWIDGETMODEL_H

#include <QAbstractTableModel>
#include <Syndication/Item>

namespace kt
{
class Feed;

QString TorrentUrlFromItem(Syndication::ItemPtr item);

/**
    @author
*/
class FeedWidgetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FeedWidgetModel(QObject *parent);
    ~FeedWidgetModel();

    Feed *currentFeed()
    {
        return feed;
    }
    void setCurrentFeed(Feed *f);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

    Syndication::ItemPtr itemForIndex(const QModelIndex &idx);

    void updated();

private:
    Feed *feed;
    QList<Syndication::ItemPtr> items;
};

}

#endif
