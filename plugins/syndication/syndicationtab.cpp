/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

#include <KToolBar>

#include <kactioncollection.h>
#include "syndicationtab.h"
#include "feedlistview.h"
#include "filterlistview.h"

namespace kt
{

    SyndicationTab::SyndicationTab(KActionCollection* ac, FeedList* feeds, FilterList* filters, QWidget* parent)
        : QWidget(parent), feeds(feeds), splitter(nullptr), filters(filters)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        splitter = new QSplitter(Qt::Vertical, this);
        layout->addWidget(splitter);

        QWidget* widget = new QWidget(splitter);
        layout = new QVBoxLayout(widget);
        layout->setSpacing(0);
        layout->setMargin(0);

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
        layout->setMargin(0);

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

    void SyndicationTab::showFeedViewMenu(const QPoint& pos)
    {
        feed_view_menu->popup(feed_view->viewport()->mapToGlobal(pos));
    }

    void SyndicationTab::showFilterViewMenu(const QPoint& pos)
    {
        filter_view_menu->popup(filter_view->viewport()->mapToGlobal(pos));
    }

    void SyndicationTab::loadState(KConfigGroup& g)
    {
        splitter->restoreState(g.readEntry("ver_splitter", QByteArray()));
    }

    void SyndicationTab::saveState(KConfigGroup& g)
    {
        g.writeEntry("ver_splitter", splitter->saveState());
    }
}
