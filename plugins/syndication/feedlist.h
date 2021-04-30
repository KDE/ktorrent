/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDLIST_H
#define KTFEEDLIST_H

#include <QAbstractListModel>
#include <QList>

namespace kt
{
class Filter;
class FilterList;
class Feed;
class SyndicationActivity;

/**
    List model which keeps track of all feeds
*/
class FeedList : public QAbstractListModel
{
public:
    FeedList(const QString &data_dir, QObject *parent);
    ~FeedList();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

    void addFeed(Feed *f);
    void loadFeeds(FilterList *filters, SyndicationActivity *activity);
    Feed *feedForIndex(const QModelIndex &idx);
    Feed *feedForDirectory(const QString &dir);
    void removeFeeds(const QModelIndexList &idx);
    void filterRemoved(Filter *f);
    void filterEdited(Filter *f);
    void importOldFeeds();

    void feedUpdated();

private:
    QList<Feed *> feeds;
    QString data_dir;
};

}

#endif
