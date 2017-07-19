/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#ifndef SYNDICATIONACTIVITY_H
#define SYNDICATIONACTIVITY_H

#include <QSplitter>
#include <Syndication/Loader>

#include <interfaces/activity.h>

namespace kt
{
    class Feed;
    class Filter;
    class FilterList;
    class FeedList;
    class SyndicationTab;
    class FeedWidget;
    class SyndicationPlugin;


    class SyndicationActivity : public kt::Activity
    {
    Q_OBJECT

    public:
        SyndicationActivity(SyndicationPlugin* sp, QWidget* parent);
        ~SyndicationActivity();

        void loadState(KSharedConfigPtr cfg);
        void saveState(KSharedConfigPtr cfg);
        Filter* addNewFilter();

        void addFeed();
        void removeFeed();
        void showFeed(Feed* f);
        void downloadLink(const QUrl& url, const QString& group, const QString& location, const QString& move_on_completion, bool silently);
        void addFilter();
        void removeFilter();
        void editFilter();
        void editFilter(Filter* f);
        void manageFilters();
        void editFeedName();

    public Q_SLOTS:
        void loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);

    private:
        FeedList* feed_list;
        FilterList* filter_list;
        SyndicationTab* tab;
        FeedWidget* feed_widget;
        QSplitter* splitter;
        QMap<Syndication::Loader*, QString> downloads;
        SyndicationPlugin* sp;
    };
}

#endif // SYNDICATIONACTIVITY_H
