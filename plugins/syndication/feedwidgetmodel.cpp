/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDateTime>
#include <QDomElement>
#include <QIcon>
#include <QLocale>

#include <KLocalizedString>

#include <Syndication/Enclosure>
#include <Syndication/Item>

#include "feedwidgetmodel.h"
#include "ktfeed.h"
#include <util/log.h>

using namespace bt;

namespace kt
{
FeedWidgetModel::FeedWidgetModel(QObject *parent)
    : QAbstractTableModel(parent)
    , feed(nullptr)
{
}

FeedWidgetModel::~FeedWidgetModel()
{
}

void FeedWidgetModel::setCurrentFeed(Feed *f)
{
    beginResetModel();
    items.clear();
    if (feed) {
        disconnect(feed, &Feed::updated, this, &FeedWidgetModel::updated);
    }

    feed = f;
    if (feed) {
        Syndication::FeedPtr ptr = feed->feedData();
        if (ptr) {
            items = ptr->items();
        }
        connect(feed, &Feed::updated, this, &FeedWidgetModel::updated);
    }
    endResetModel();
}

int FeedWidgetModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return items.count();
    } else {
        return 0;
    }
}

int FeedWidgetModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return NUM_COLUMNS;
    } else {
        return 0;
    }
}

QVariant FeedWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || section < 0 || section >= NUM_COLUMNS || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (Column{section}) {
    case Column::TITLE:
        return i18n("Title");
    case Column::DATE_PUBLISHED:
        return i18n("Date Published");
    case Column::TORRENT:
        return i18n("Torrent");
    default:
        return QVariant();
    }
}

QVariant FeedWidgetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !feed) {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= items.count()) {
        return QVariant();
    }

    const Column column{index.column()};
    Syndication::ItemPtr item = items.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (column) {
        case Column::TITLE:
            return item->title();
        case Column::DATE_PUBLISHED:
            return QLocale().toString(QDateTime::fromSecsSinceEpoch(item->datePublished()), QLocale::ShortFormat);
        case Column::TORRENT:
            return TorrentUrlFromItem(item);
        default:
            return QVariant();
        }
    } else if (role == Qt::DecorationRole && column == Column::TITLE && feed->downloaded(item)) {
        return QIcon::fromTheme(QStringLiteral("go-down"));
    } else if (role == Qt::DecorationRole && column == Column::TITLE && feed->failed(item)) {
        return QIcon::fromTheme(QStringLiteral("data-error"));
    }

    return QVariant();
}

bool FeedWidgetModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

bool FeedWidgetModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

Syndication::ItemPtr FeedWidgetModel::itemForIndex(const QModelIndex &index)
{
    if (index.row() < 0 || index.row() >= items.count()) {
        return Syndication::ItemPtr();
    }

    return items.at(index.row());
}

QString TorrentUrlFromItem(Syndication::ItemPtr item)
{
    const QList<Syndication::EnclosurePtr> encs = item->enclosures();
    for (const Syndication::EnclosurePtr &e : encs) {
        if (e->type() == QStringLiteral("application/x-bittorrent") || e->url().endsWith(QStringLiteral(".torrent"))) {
            return e->url();
        }
    }

    // Search for magnets on item's link as some RSS feeds only use this item to post the magnet link.
    QString link = item->link();
    if (!link.isEmpty()) {
        // Note that syndication library prepends the channel link to the item link by default, so
        // we need to extract the magnet from the string.
        int magnetStartIndex = link.indexOf(QStringLiteral("magnet:"));
        if (magnetStartIndex >= 0) {
            return link.right(link.size() - magnetStartIndex);
        }
    }

    QMultiMap<QString, QDomElement> props = item->additionalProperties();
    QMultiMap<QString, QDomElement>::iterator itr = props.begin();
    while (itr != props.end()) {
        const QDomElement &elem = itr.value();
        if (elem.nodeName() == QStringLiteral("torrent")) {
            QDomElement uri = elem.firstChildElement(QStringLiteral("magnetURI"));
            if (!uri.isNull()) {
                return uri.text();
            }
        }
        itr++;
    }

    return QString();
}

void FeedWidgetModel::updated()
{
    if (!feed) {
        return;
    }

    beginResetModel();
    items.clear();
    Syndication::FeedPtr ptr = feed->feedData();
    if (ptr) {
        items = ptr->items();
    }
    endResetModel();
}
}

#include "moc_feedwidgetmodel.cpp"
