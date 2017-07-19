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

#ifndef KTFEEDWIDGET_H
#define KTFEEDWIDGET_H

#include <QWidget>
#include <KConfigGroup>

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
        FeedWidget(FilterList* filters, SyndicationActivity* act, QWidget* parent);
        ~FeedWidget();

        /// Set the Feed to show, can be 0
        void setFeed(Feed* feed);

        Feed* getFeed() {return feed;}

        void loadState(KConfigGroup& g);
        void saveState(KConfigGroup& g);

        void downloadClicked();
        void refreshClicked();
        void filtersClicked();
        void cookiesClicked();
        void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void updated();
        void onFeedRenamed(Feed* f);
        void refreshRateChanged(int v);
        void linkClicked(const QUrl& url);
        void resizeColumns();

    Q_SIGNALS:
        void updateCaption(QWidget* w, const QString& text);

    private:
        Feed* feed;
        FeedWidgetModel* model;
        FilterList* filters;
        SyndicationActivity* act;

        static QString item_template;
    };

}

#endif
