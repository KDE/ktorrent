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
        Q_OBJECT
    public:
        FeedList(const QString& data_dir, QObject* parent);
        virtual ~FeedList();

        virtual int rowCount(const QModelIndex& parent) const;
        virtual QVariant data(const QModelIndex& index, int role) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual bool removeRows(int row, int count, const QModelIndex& parent);
        virtual bool insertRows(int row, int count, const QModelIndex& parent);

        void addFeed(Feed* f);
        void loadFeeds(FilterList* filters, SyndicationActivity* activity);
        Feed* feedForIndex(const QModelIndex& idx);
        Feed* feedForDirectory(const QString& dir);
        void removeFeeds(const QModelIndexList& idx);
        void filterRemoved(Filter* f);
        void filterEdited(Filter* f);
        void importOldFeeds();

    private slots:
        void feedUpdated();

    private:
        QList<Feed*> feeds;
        QString data_dir;
    };

}

#endif
