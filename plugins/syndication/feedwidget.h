/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDWIDGET_H
#define KTFEEDWIDGET_H

#include <KConfigGroup>
#include <QWidget>

#include "ui_feedwidget.h"

namespace kt
{
class Feed;
class FeedWidgetModel;
class FilterList;
class SyndicationActivity;

/**
    Displays a Feed
*/
class FeedWidget : public QWidget, public Ui_FeedWidget
{
    Q_OBJECT
public:
    FeedWidget(FilterList *filters, SyndicationActivity *act, QWidget *parent);
    ~FeedWidget();

    /// Set the Feed to show, can be 0
    void setFeed(Feed *feed);

    Feed *getFeed()
    {
        return feed;
    }

    void loadState(KConfigGroup &g);
    void saveState(KConfigGroup &g);

    void downloadClicked();
    void refreshClicked();
    void filtersClicked();
    void cookiesClicked();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void updated();
    void onFeedRenamed(Feed *f);
    void refreshRateChanged(int v);
    void resizeColumns();

Q_SIGNALS:
    void updateCaption(QWidget *w, const QString &text);

private:
    Feed *feed;
    FeedWidgetModel *model;
    FilterList *filters;
    SyndicationActivity *act;

    static QString item_template;
};

}

#endif
