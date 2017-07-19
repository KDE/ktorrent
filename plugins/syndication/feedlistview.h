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
        FeedListView(FeedList* feeds, QWidget* parent);
        ~FeedListView();

        /// Get all the selected feeds
        QModelIndexList selectedFeeds();

        void itemActivated(const QModelIndex& idx);
        void selectionChanged(const QItemSelection& sel, const QItemSelection& desel);

    Q_SIGNALS:
        void feedActivated(Feed* feed);
        void enableRemove(bool on);

    private:
        FeedList* feeds;
    };

}

#endif
