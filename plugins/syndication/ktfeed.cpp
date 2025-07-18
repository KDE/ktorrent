/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDomElement>
#include <QFile>

#include <KLocalizedString>
#include <Syndication/Enclosure>

#include "feedretriever.h"
#include "filter.h"
#include "filterlist.h"
#include "ktfeed.h"
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
const int DEFAULT_REFRESH_RATE = 60;

Feed::Feed(const QString &dir)
    : dir(dir)
    , status(UNLOADED)
    , refresh_rate(DEFAULT_REFRESH_RATE)
{
    connect(&update_timer, &QTimer::timeout, this, &Feed::refresh);
}

Feed::Feed(const QString &feed_url, const QString &dir)
    : dir(dir)
    , status(UNLOADED)
    , refresh_rate(DEFAULT_REFRESH_RATE)
{
    parseUrl(feed_url);
    connect(&update_timer, &QTimer::timeout, this, &Feed::refresh);
    refresh();
    save();
}

Feed::Feed(const QString &feed_url, Syndication::FeedPtr feed, const QString &dir)
    : feed(feed)
    , dir(dir)
    , status(OK)
    , refresh_rate(DEFAULT_REFRESH_RATE)
{
    parseUrl(feed_url);
    connect(&update_timer, &QTimer::timeout, this, &Feed::refresh);
    update_timer.start(refresh_rate * 60 * 1000);
}

Feed::~Feed()
{
}

void Feed::parseUrl(const QString &feed_url)
{
    QStringList sl = feed_url.split(QStringLiteral(":COOKIE:"));
    if (sl.size() == 2) {
        url = QUrl(sl.first());
        cookie = sl.last();
    } else
        url = QUrl(feed_url);
}

void Feed::save()
{
    QString file = dir + QStringLiteral("info");
    File fptr;
    if (!fptr.open(file, QStringLiteral("wt"))) {
        Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    BEncoder enc(&fptr);
    enc.beginDict();
    enc.write(QByteArrayLiteral("url"));
    enc.write(url.toEncoded());
    if (!cookie.isEmpty()) {
        enc.write(QByteArrayLiteral("cookie"));
        enc.write(cookie.toUtf8());
    }
    enc.write(QByteArrayLiteral("filters"));
    enc.beginList();
    for (Filter *f : std::as_const(filters))
        enc.write(f->filterID().toUtf8());
    enc.end();
    enc.write(QByteArrayLiteral("loaded"));
    enc.beginList();
    for (const QString &id : std::as_const(loaded))
        enc.write(id.toUtf8());
    enc.end();
    enc.write(QByteArrayLiteral("downloaded_se_items"));
    enc.beginList();
    QMap<Filter *, QList<SeasonEpisodeItem>>::iterator i = downloaded_se_items.begin();
    while (i != downloaded_se_items.end()) {
        Filter *f = i.key();
        const QList<SeasonEpisodeItem> &se = i.value();
        enc.write(f->filterID().toUtf8());
        enc.beginList();
        for (const SeasonEpisodeItem &item : se) {
            enc.write((bt::Uint32)item.season);
            enc.write((bt::Uint32)item.episode);
        }
        enc.end();
        i++;
    }
    enc.end();
    if (!custom_name.isEmpty())
        enc.write(QByteArrayLiteral("custom_name"), custom_name.toUtf8());
    enc.write(QByteArrayLiteral("refresh_rate"), refresh_rate);
    enc.end();
}

void Feed::load(FilterList *filter_list)
{
    QString file = dir + QStringLiteral("info");
    QFile fptr(file);
    if (!fptr.open(QIODevice::ReadOnly)) {
        Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    QByteArray data = fptr.readAll();
    BDecoder dec(data, false);
    const std::unique_ptr<BDictNode> dict = dec.decodeDict();
    if (!dict) {
        return;
    }

    try {
        url = QUrl(dict->getString("url"));
        cookie = dict->getValue("cookie") ? dict->getString("cookie") : QString();
        custom_name = dict->getValue("custom_name") ? dict->getString("custom_name") : QString();
        refresh_rate = dict->getValue("refresh_rate") ? dict->getInt("refresh_rate") : DEFAULT_REFRESH_RATE;

        BListNode *fl = dict->getList("filters");
        if (fl) {
            for (Uint32 i = 0; i < fl->getNumChildren(); i++) {
                Filter *f = filter_list->filterByID(fl->getString(i));
                if (f)
                    filters.append(f);
            }
        }

        BListNode *ll = dict->getList("loaded");
        if (ll) {
            for (Uint32 i = 0; i < ll->getNumChildren(); i++) {
                loaded.insert(ll->getString(i));
            }
        }

        BListNode *se_list = dict->getList("downloaded_se_items");
        if (se_list) {
            for (Uint32 i = 0; i < se_list->getNumChildren(); i += 2) {
                BListNode *se = se_list->getList(i + 1);
                if (!se)
                    continue;

                Filter *f = filter_list->filterByID(se_list->getString(i));
                if (!f)
                    continue;

                QList<SeasonEpisodeItem> &sel = downloaded_se_items[f];
                for (Uint32 j = 0; j < se->getNumChildren(); j += 2) {
                    SeasonEpisodeItem item;
                    item.episode = se->getInt(j);
                    item.season = se->getInt(j + 1);
                    sel.append(item);
                }
            }
        }
    } catch (...) {
        throw;
    }

    Out(SYS_SYN | LOG_DEBUG) << "Loaded feed from " << file << " : " << endl;
    status = OK;

    if (bt::Exists(dir + QStringLiteral("feed.xml")))
        loadFromDisk();
    else
        refresh();
}

void Feed::loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
{
    Q_UNUSED(loader);
    if (status != Syndication::Success) {
        update_error = SyndicationErrorString(status);
        Out(SYS_SYN | LOG_NOTICE) << "Failed to load feed " << url.toDisplayString() << ": " << update_error << endl;
        this->status = FAILED_TO_DOWNLOAD;
        update_timer.start(refresh_rate * 60 * 1000);
        Q_EMIT updated();
        return;
    }

    Out(SYS_SYN | LOG_NOTICE) << "Loaded feed " << url.toDisplayString() << endl;
    this->feed = feed;
    update_timer.start(refresh_rate * 60 * 1000);
    this->status = OK;

    // refresh cache of feed_items_ids
    feed_items_id.clear();
    const QList<Syndication::ItemPtr> feedItems = feed->items();
    for (const Syndication::ItemPtr &item : feedItems)
        feed_items_id.insert(item->id());

    checkLoaded();
    runFilters();
    Q_EMIT updated();
}

void Feed::refresh()
{
    status = DOWNLOADING;
    update_error.clear();
    update_timer.stop();
    Syndication::Loader *loader = Syndication::Loader::create(this, SLOT(loadingComplete(Syndication::Loader *, Syndication::FeedPtr, Syndication::ErrorCode)));
    FeedRetriever *retr = new FeedRetriever(dir + QStringLiteral("feed.xml"));
    if (!cookie.isEmpty())
        retr->setAuthenticationCookie(cookie);
    loader->loadFrom(url, retr);
    Q_EMIT updated();
}

void Feed::loadingFromDiskComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
{
    loadingComplete(loader, feed, status);
    refresh();
}

void Feed::loadFromDisk()
{
    status = DOWNLOADING;
    update_timer.stop();
    Syndication::Loader *loader =
        Syndication::Loader::create(this, SLOT(loadingFromDiskComplete(Syndication::Loader *, Syndication::FeedPtr, Syndication::ErrorCode)));
    loader->loadFrom(QUrl::fromLocalFile(dir + QStringLiteral("feed.xml")), new FeedRetriever());
    Q_EMIT updated();
}

QString Feed::title() const
{
    if (feed)
        return feed->title();
    else
        return url.toDisplayString();
}

QString Feed::newFeedDir(const QString &base)
{
    int cnt = 0;
    QString dir = QStringLiteral("%1feed%2/").arg(base).arg(cnt);
    while (bt::Exists(dir)) {
        cnt++;
        dir = QStringLiteral("%1feed%2/").arg(base).arg(cnt);
    }

    bt::MakeDir(dir);
    return dir;
}

void Feed::addFilter(Filter *f)
{
    filters.append(f);
    Q_EMIT updated();
}

void Feed::removeFilter(Filter *f)
{
    filters.removeAll(f);
    downloaded_se_items.remove(f);
    Q_EMIT updated();
}

bool Feed::needToDownload(Syndication::ItemPtr item, Filter *filter)
{
    bool m = filter->match(item);
    if ((m && filter->downloadMatching()) || (!m && filter->downloadNonMatching())) {
        if (filter->useSeasonAndEpisodeMatching() && filter->noDuplicateSeasonAndEpisodeMatches()) {
            int s = 0;
            int e = 0;
            Filter::getSeasonAndEpisode(item->title(), s, e);
            if (!downloaded_se_items.contains(filter)) {
                downloaded_se_items[filter].append(SeasonEpisodeItem(s, e));
            } else {
                // If we have already downloaded this season and episode, return
                QList<SeasonEpisodeItem> &ses = downloaded_se_items[filter];
                SeasonEpisodeItem se(s, e);
                if (ses.contains(se))
                    return false;

                ses.append(se);
            }
        }

        return true;
    }

    return false;
}

void Feed::runFilters()
{
    if (!feed)
        return;

    Out(SYS_SYN | LOG_NOTICE) << "Running filters on " << feed->title() << endl;
    for (Filter *f : std::as_const(filters)) {
        f->startMatching();
        const QList<Syndication::ItemPtr> items = feed->items();
        for (const Syndication::ItemPtr &item : items) {
            // Skip already loaded items
            if (loaded.contains(item->id()))
                continue;

            if (needToDownload(item, f)) {
                Out(SYS_SYN | LOG_NOTICE) << "Downloading item " << item->title() << " (filter: " << f->filterName() << ")" << endl;
                downloadItem(item, f->group(), f->downloadLocation(), f->moveOnCompletionLocation(), f->openSilently());
            }
        }
    }
}

QString TorrentUrlFromItem(Syndication::ItemPtr item);

void Feed::downloadItem(Syndication::ItemPtr item, const QString &group, const QString &location, const QString &move_on_completion, bool silently)
{
    loaded.insert(item->id());
    QString url = TorrentUrlFromItem(item);
    if (!url.isEmpty())
        Q_EMIT downloadLink(QUrl(url), group, location, move_on_completion, silently);
    else
        Q_EMIT downloadLink(QUrl(item->link()), group, location, move_on_completion, silently);
    save();
}

void Feed::clearFilters()
{
    filters.clear();
    Q_EMIT updated();
}

void Feed::checkLoaded()
{
    // remove all id's which are in loaded but no longer in
    // the item list
    bool need_to_save = false;

    QList<QString> itemsToRemove;
    for (const QString &loadedItem : std::as_const(loaded)) {
        if (!feed_items_id.contains(loadedItem)) {
            itemsToRemove.push_front(loadedItem);
            need_to_save = true;
        }
    }

    for (const QString &itemToRemove : std::as_const(itemsToRemove))
        loaded.remove(itemToRemove);

    if (need_to_save)
        save();
}

bool Feed::downloaded(Syndication::ItemPtr item) const
{
    return loaded.contains(item->id());
}

QString Feed::displayName() const
{
    if (!custom_name.isEmpty())
        return custom_name;
    else if (ok())
        return feed->title();
    else
        return url.toDisplayString();
}

void Feed::setDisplayName(const QString &dname)
{
    if (custom_name != dname) {
        custom_name = dname;
        save();
        Q_EMIT feedRenamed(this);
    }
}

void Feed::setRefreshRate(bt::Uint32 r)
{
    if (r > 0) {
        refresh_rate = r;
        save();
        update_timer.setInterval(refresh_rate * 60 * 1000);
    }
}

//////////////////////////////////
SeasonEpisodeItem::SeasonEpisodeItem()
    : season(-1)
    , episode(-1)
{
}

SeasonEpisodeItem::SeasonEpisodeItem(int s, int e)
    : season(s)
    , episode(e)
{
}

SeasonEpisodeItem::SeasonEpisodeItem(const SeasonEpisodeItem &item)
    : season(item.season)
    , episode(item.episode)
{
}

bool SeasonEpisodeItem::operator==(const SeasonEpisodeItem &item) const
{
    return season == item.season && episode == item.episode;
}

SeasonEpisodeItem &SeasonEpisodeItem::operator=(const SeasonEpisodeItem &item)
{
    season = item.season;
    episode = item.episode;
    return *this;
}

/////////////////////////////////////
QString SyndicationErrorString(Syndication::ErrorCode err)
{
    switch (err) {
    case Syndication::Aborted:
        return i18n("Aborted");
    case Syndication::Timeout:
        return i18n("Timeout when downloading feed");
    case Syndication::UnknownHost:
        return i18n("Unknown hostname");
    case Syndication::FileNotFound:
        return i18n("File not found");
    case Syndication::OtherRetrieverError:
        return i18n("Unknown retriever error");
    case Syndication::InvalidXml:
    case Syndication::XmlNotAccepted:
    case Syndication::InvalidFormat:
        return i18n("Invalid feed data");
    case Syndication::Success:
        return i18n("Success");
    default:
        return QString();
    }
}

QString Feed::filterNamesString() const
{
    if (filters.empty())
        return i18n("None");
    QStringList names;
    for (Filter *f : std::as_const(filters))
        names << f->filterName();
    return names.join(QStringLiteral(", "));
}

}

#include "moc_ktfeed.cpp"
