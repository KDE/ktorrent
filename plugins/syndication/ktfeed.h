/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEED_H
#define KTFEED_H

#include <QMap>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include <Syndication/Feed>
#include <Syndication/Loader>

#include <util/constants.h>

namespace kt
{
class Filter;
class FilterList;

struct SeasonEpisodeItem {
    int season;
    int episode;

    SeasonEpisodeItem();
    SeasonEpisodeItem(int s, int e);
    SeasonEpisodeItem(const SeasonEpisodeItem &item);

    bool operator==(const SeasonEpisodeItem &item) const;
    SeasonEpisodeItem &operator=(const SeasonEpisodeItem &item);
};

/// Convert a syndication error into an error string
QString SyndicationErrorString(Syndication::ErrorCode err);

/**
    Class to keep track of a feed.
*/
class Feed : public QObject
{
    Q_OBJECT
public:
    Feed(const QString &dir);
    Feed(const QString &feed_url, const QString &dir);
    Feed(const QString &feed_url, Syndication::FeedPtr feed, const QString &dir);
    ~Feed();

    enum Status {
        UNLOADED,
        OK,
        FAILED_TO_DOWNLOAD,
        DOWNLOADING,
    };

    /// Get the display name of the feed
    QString displayName() const;

    /// Set the display name
    void setDisplayName(const QString &dname);

    /// Get the libsyndication feed
    Syndication::FeedPtr feedData()
    {
        return feed;
    }

    /// Get the URL of the feed
    QUrl feedUrl() const
    {
        return url;
    }

    /// Get the authentication cookie
    const QString &authenticationCookie() const
    {
        return cookie;
    }

    /// Set the authentication cookie
    void setAuthenticationCookie(const QString &nc)
    {
        cookie = nc;
    }

    /// Is the feed OK
    bool ok() const
    {
        return !feed.isNull();
    }

    /// Save the feed to it's directory
    void save();

    /// Load the feed from it's directory
    void load(FilterList *filter_list);

    /// Get the feed's data directory
    QString directory() const
    {
        return dir;
    }

    /// Get the current status of the feed
    Status feedStatus() const
    {
        return status;
    }

    /// Get the tile of the feed
    QString title() const;

    /// Get the update error string
    QString errorString() const
    {
        return update_error;
    }

    /// Create a new feed directory
    static QString newFeedDir(const QString &base);

    /// Add a filter to the feed
    void addFilter(Filter *f);

    /// Remove a filter from the feed
    void removeFilter(Filter *f);

    /// Run filters on the feed
    void runFilters();

    /// See if the feed is using a filter
    bool usingFilter(Filter *f) const
    {
        return filters.contains(f);
    }

    /// Clear all filters
    void clearFilters();

    /// Download an item from the feed
    void downloadItem(Syndication::ItemPtr item, const QString &group, const QString &location, const QString &move_on_completion, bool silently);

    /// Check if an item is downloaded
    bool downloaded(Syndication::ItemPtr item) const;

    /// Get the number of filters
    int numFilters() const
    {
        return filters.count();
    }

    /// Get a comma separated string of the filter names
    QString filterNamesString() const;

    /// Get the refresh rate (in minutes) of the feed
    bt::Uint32 refreshRate() const
    {
        return refresh_rate;
    }

    /// Set the refresh rate of the feed
    void setRefreshRate(bt::Uint32 r);

Q_SIGNALS:
    /// Emitted when a link must de downloaded
    void downloadLink(const QUrl &link, const QString &group, const QString &location, const QString &move_on_completion, bool silently);

    /// A feed has been renamed
    void feedRenamed(Feed *f);

public:
    /// Update the feed
    void refresh();

private Q_SLOTS:
    void loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);
    void loadingFromDiskComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);

Q_SIGNALS:
    void updated();

private:
    bool needToDownload(Syndication::ItemPtr item, Filter *filter);
    void checkLoaded();
    void loadFromDisk();
    void parseUrl(const QString &feed_url);

private:
    QUrl url;
    Syndication::FeedPtr feed;
    QSet<QString> feed_items_id;
    QString dir;
    QTimer update_timer;
    Status status;
    QList<Filter *> filters;
    QSet<QString> loaded;
    QMap<Filter *, QList<SeasonEpisodeItem>> downloaded_se_items;
    QString custom_name;
    bt::Uint32 refresh_rate;
    QString cookie;
    QString update_error;
};

}

#endif
