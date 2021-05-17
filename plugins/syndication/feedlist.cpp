/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTextStream>
#include <QTime>

#include <KLocalizedString>

#include "feedlist.h"
#include "filterlist.h"
#include "ktfeed.h"
#include "syndicationactivity.h"
#include <interfaces/functions.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
FeedList::FeedList(const QString &data_dir, QObject *parent)
    : QAbstractListModel(parent)
    , data_dir(data_dir)
{
}

FeedList::~FeedList()
{
    qDeleteAll(feeds);
}

void FeedList::loadFeeds(FilterList *filter_list, SyndicationActivity *activity)
{
    QDir dir(data_dir);
    QStringList filters;
    filters << QStringLiteral("feed*");
    const QStringList sl = dir.entryList(filters, QDir::Dirs);
    for (const QString &s : sl) {
        QString idir = data_dir + s;
        if (!idir.endsWith(DirSeparator()))
            idir.append(DirSeparator());

        Out(SYS_GEN | LOG_NOTICE) << "Loading feed from directory " << idir << endl;
        Feed *feed = 0;
        try {
            feed = new Feed(idir);
            connect(feed, &Feed::downloadLink, activity, &SyndicationActivity::downloadLink);
            feed->load(filter_list);
            addFeed(feed);

        } catch (...) {
            delete feed;
        }
    }
}

void FeedList::importOldFeeds()
{
    QFile fptr(kt::DataDir() + QStringLiteral("rssfeeds.ktr"));
    if (!fptr.open(QIODevice::ReadOnly))
        return;

    QDataStream in(&fptr);
    int num_feeds;
    in >> num_feeds;
    for (int i = 0; i < num_feeds; i++) {
        QUrl feed_url;
        QString title;
        int active;
        int article_age;
        int ignore_ttl;
        QTime auto_refresh;
        QString protocol;
        QString user;
        QString pass;
        QString host;
        QString path;
        QString path_encoded;
        QString query;
        QString ref_encoded;
        unsigned char malf;
        unsigned short port;

        in >> protocol >> user >> pass >> host >> path >> path_encoded >> query >> ref_encoded >> malf >> port;
        feed_url.setScheme(protocol);
        feed_url.setUserName(user);
        feed_url.setPassword(pass);
        feed_url.setHost(host);
        feed_url.setPath(path);
        feed_url.setQuery(query);
        feed_url.setFragment(ref_encoded);
        feed_url.setPort(port == 0 ? -1 : port);

        in >> title >> active >> article_age >> ignore_ttl >> auto_refresh;
        Out(SYS_GEN | LOG_DEBUG) << "Importing " << feed_url.toDisplayString() << " ..." << endl;

        // check for duplicate URL's
        bool found = false;
        for (Feed *f : qAsConst(feeds)) {
            if (f->feedUrl() == feed_url) {
                found = true;
                break;
            }
        }

        if (!found) {
            Feed *f = new Feed(feed_url.toString(), Feed::newFeedDir(data_dir));
            addFeed(f);
        }
    }

    fptr.close();
    // move the rssfeeds.ktr file to a backup location
    // so that it doesn't get immorted twice
    bt::Move(kt::DataDir() + QStringLiteral("rssfeeds.ktr"), kt::DataDir() + QStringLiteral("imported-rssfeeds.ktr"), true, true);
}

void FeedList::addFeed(Feed *f)
{
    feeds.append(f);
    connect(f, &Feed::updated, this, &FeedList::feedUpdated);
    insertRow(feeds.count() - 1);
}

int FeedList::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? feeds.count() : 0;
}

QVariant FeedList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Feed *f = feeds.at(index.row());
    if (!f->feedData())
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return f->displayName();
    case Qt::UserRole:
        return i18np("%2\n1 active filter", "%2\n%1 active filters", f->numFilters(), f->displayName());
    case Qt::DecorationRole:
        if (f->feedStatus() == Feed::FAILED_TO_DOWNLOAD)
            return QIcon::fromTheme(QStringLiteral("dialog-error"));
        else
            return QIcon::fromTheme(QStringLiteral("application-rss+xml"));
    case Qt::ToolTipRole:
        if (f->feedStatus() == Feed::FAILED_TO_DOWNLOAD)
            return i18n("<b>%1</b><br/><br/>Download failed: <b>%2</b>", f->feedData()->link(), f->errorString());
        else if (f->ok())
            return i18n("<b>%1</b><br/><br/>%2", f->feedData()->link(), f->feedData()->description());
        break;
    }

    return QVariant();
}

bool FeedList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || !value.canConvert<QString>())
        return false;

    Feed *f = feeds.at(index.row());
    f->setDisplayName(value.toString());
    Q_EMIT dataChanged(index, index);
    return true;
}

Qt::ItemFlags FeedList::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

Feed *FeedList::feedForIndex(const QModelIndex &idx)
{
    if (!idx.isValid())
        return 0;

    return feeds.at(idx.row());
}

Feed *FeedList::feedForDirectory(const QString &dir)
{
    for (Feed *f : qAsConst(feeds))
        if (f->directory() == dir)
            return f;

    return 0;
}

bool FeedList::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

bool FeedList::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

void FeedList::removeFeeds(const QModelIndexList &idx)
{
    QList<Feed *> to_remove;
    for (const QModelIndex &i : idx) {
        Feed *f = feedForIndex(i);
        if (f)
            to_remove.append(f);
    }

    beginResetModel();
    for (Feed *f : qAsConst(to_remove)) {
        bt::Delete(f->directory(), true);
        feeds.removeAll(f);
        delete f;
    }
    endResetModel();
}

void FeedList::feedUpdated()
{
    Feed *f = (Feed *)sender();
    int idx = feeds.indexOf(f);
    if (idx >= 0)
        Q_EMIT dataChanged(index(idx, 0), index(idx, 0));
}

void FeedList::filterRemoved(Filter *f)
{
    for (Feed *feed : qAsConst(feeds))
        feed->removeFilter(f);
}

void FeedList::filterEdited(Filter *f)
{
    for (Feed *feed : qAsConst(feeds)) {
        if (feed->usingFilter(f))
            feed->runFilters();
    }
}
}
