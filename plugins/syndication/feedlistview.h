/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDLISTVIEW_H
#define KTFEEDLISTVIEW_H

#include <QListView>

namespace kt
{
class Feed;
class FeedList;

/**
    View to show the list of feeds.
*/
class FeedListView : public QListView
{
    Q_OBJECT
public:
    FeedListView(FeedList *feeds, QWidget *parent);
    ~FeedListView();

    /// Get all the selected feeds
    QModelIndexList selectedFeeds();

    void itemActivated(const QModelIndex &idx);
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;

Q_SIGNALS:
    void feedActivated(Feed *feed);
    void enableRemove(bool on);

private:
    FeedList *feeds;
};

}

#endif
