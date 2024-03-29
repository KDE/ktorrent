/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

#include <KToolBar>

#include "feedlistview.h"
#include "filterlistview.h"
#include "syndicationtab.h"
#include <kactioncollection.h>

namespace kt
{
SyndicationTab::SyndicationTab(KActionCollection *ac, FeedList *feeds, FilterList *filters, QWidget *parent)
    : QWidget(parent)
    , feeds(feeds)
    , splitter(nullptr)
    , filters(filters)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    QWidget *widget = new QWidget(splitter);
    layout = new QVBoxLayout(widget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    feed_tool_bar = new KToolBar(widget);
    feed_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    feed_tool_bar->addAction(ac->action(QStringLiteral("add_feed")));
    feed_tool_bar->addAction(ac->action(QStringLiteral("remove_feed")));
    feed_tool_bar->addSeparator();
    feed_tool_bar->addAction(ac->action(QStringLiteral("manage_filters")));
    layout->addWidget(feed_tool_bar);

    feed_view = new FeedListView(feeds, widget);
    layout->addWidget(feed_view);
    splitter->addWidget(widget);

    widget = new QWidget(splitter);
    layout = new QVBoxLayout(widget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    filter_tool_bar = new KToolBar(widget);
    filter_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    filter_tool_bar->addAction(ac->action(QStringLiteral("add_filter")));
    filter_tool_bar->addAction(ac->action(QStringLiteral("remove_filter")));
    filter_tool_bar->addSeparator();
    filter_tool_bar->addAction(ac->action(QStringLiteral("edit_filter")));
    layout->addWidget(filter_tool_bar);

    filter_view = new FilterListView(filters, widget);
    layout->addWidget(filter_view);
    splitter->addWidget(widget);

    feed_view_menu = new QMenu(this);
    feed_view_menu->addAction(ac->action(QStringLiteral("manage_filters")));
    feed_view_menu->addAction(ac->action(QStringLiteral("edit_feed_name")));
    feed_view_menu->addSeparator();
    feed_view_menu->addAction(ac->action(QStringLiteral("add_feed")));
    feed_view_menu->addAction(ac->action(QStringLiteral("remove_feed")));
    connect(feed_view, &FeedListView::customContextMenuRequested, this, &SyndicationTab::showFeedViewMenu);

    filter_view_menu = new QMenu(this);
    filter_view_menu->addAction(ac->action(QStringLiteral("edit_filter")));
    filter_view_menu->addSeparator();
    filter_view_menu->addAction(ac->action(QStringLiteral("add_filter")));
    filter_view_menu->addAction(ac->action(QStringLiteral("remove_filter")));
    connect(filter_view, &FilterListView::customContextMenuRequested, this, &SyndicationTab::showFilterViewMenu);
}

SyndicationTab::~SyndicationTab()
{
}

void SyndicationTab::showFeedViewMenu(const QPoint &pos)
{
    feed_view_menu->popup(feed_view->viewport()->mapToGlobal(pos));
}

void SyndicationTab::showFilterViewMenu(const QPoint &pos)
{
    filter_view_menu->popup(filter_view->viewport()->mapToGlobal(pos));
}

void SyndicationTab::loadState(KConfigGroup &g)
{
    splitter->restoreState(g.readEntry("ver_splitter", QByteArray()));
}

void SyndicationTab::saveState(KConfigGroup &g)
{
    g.writeEntry("ver_splitter", splitter->saveState());
}
}
