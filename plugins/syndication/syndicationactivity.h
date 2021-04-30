/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    SyndicationActivity(SyndicationPlugin *sp, QWidget *parent);
    ~SyndicationActivity();

    void loadState(KSharedConfigPtr cfg);
    void saveState(KSharedConfigPtr cfg);
    Filter *addNewFilter();

    void addFeed();
    void removeFeed();
    void showFeed(Feed *f);
    void downloadLink(const QUrl &url, const QString &group, const QString &location, const QString &move_on_completion, bool silently);
    void addFilter();
    void removeFilter();
    void editFilter();
    void editFilter(Filter *f);
    void manageFilters();
    void editFeedName();

public Q_SLOTS:
    void loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);

private:
    FeedList *feed_list;
    FilterList *filter_list;
    SyndicationTab *tab;
    FeedWidget *feed_widget;
    QSplitter *splitter;
    QMap<Syndication::Loader *, QString> downloads;
    SyndicationPlugin *sp;
};
}

#endif // SYNDICATIONACTIVITY_H
