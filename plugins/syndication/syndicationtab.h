/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSYNDICATIONTAB_H
#define KTSYNDICATIONTAB_H

#include <KConfigGroup>
#include <QWidget>

class QSplitter;
class KToolBar;
class KActionCollection;
class QMenu;

namespace kt
{
class FeedList;
class FeedListView;
class FilterList;
class FilterListView;

/**
    @author
*/
class SyndicationTab : public QWidget
{
public:
    SyndicationTab(KActionCollection *ac, FeedList *feeds, FilterList *filters, QWidget *parent);
    ~SyndicationTab();

    FeedListView *feedView()
    {
        return feed_view;
    }
    FilterListView *filterView()
    {
        return filter_view;
    }
    void loadState(KConfigGroup &g);
    void saveState(KConfigGroup &g);

    void showFeedViewMenu(const QPoint &pos);
    void showFilterViewMenu(const QPoint &pos);

private:
    FeedList *feeds;
    FeedListView *feed_view;
    KToolBar *feed_tool_bar;
    QSplitter *splitter;
    FilterList *filters;
    FilterListView *filter_view;
    KToolBar *filter_tool_bar;
    QMenu *feed_view_menu;
    QMenu *filter_view_menu;
};

}

#endif
