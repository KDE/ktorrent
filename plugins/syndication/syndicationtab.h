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

#ifndef KTSYNDICATIONTAB_H
#define KTSYNDICATIONTAB_H

#include <QWidget>
#include <KConfigGroup>

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
        SyndicationTab(KActionCollection* ac, FeedList* feeds, FilterList* filters, QWidget* parent);
        ~SyndicationTab();

        FeedListView* feedView() {return feed_view;}
        FilterListView* filterView() {return filter_view;}
        void loadState(KConfigGroup& g);
        void saveState(KConfigGroup& g);

        void showFeedViewMenu(const QPoint& pos);
        void showFilterViewMenu(const QPoint& pos);

    private:
        FeedList* feeds;
        FeedListView* feed_view;
        KToolBar* feed_tool_bar;
        QSplitter* splitter;
        FilterList* filters;
        FilterListView* filter_view;
        KToolBar* filter_tool_bar;
        QMenu* feed_view_menu;
        QMenu* filter_view_menu;
    };

}

#endif
