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
#include <QDateTime>
#include <QDomElement>
#include <QIcon>
#include <QLocale>
#include <klocalizedstring.h>
#include <syndication/item.h>
#include <syndication/enclosure.h>
#include <util/log.h>
#include "feedwidgetmodel.h"
#include "ktfeed.h"

using namespace bt;

namespace kt
{

    FeedWidgetModel::FeedWidgetModel(QObject* parent)
        : QAbstractTableModel(parent),
          feed(0)
    {
    }


    FeedWidgetModel::~FeedWidgetModel()
    {
    }

    void FeedWidgetModel::setCurrentFeed(Feed* f)
    {
        items.clear();
        if (feed)
            disconnect(feed, SIGNAL(updated()), this, SLOT(updated()));

        feed = f;
        if (feed)
        {
            Syndication::FeedPtr ptr = feed->feedData();
            if (ptr)
                items = ptr->items();
            connect(feed, SIGNAL(updated()), this, SLOT(updated()));
            reset();
        }
    }

    int FeedWidgetModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return items.count();
        else
            return 0;
    }

    int FeedWidgetModel::columnCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return 3;
        else
            return 0;
    }

    QVariant FeedWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || section < 0 || section >= 3 || orientation != Qt::Horizontal)
            return QVariant();

        switch (section)
        {
        case 0: return i18n("Title");
        case 1: return i18n("Date Published");
        case 2: return i18n("Torrent");
        default:
            return QVariant();
        }
    }

    QVariant FeedWidgetModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || !feed)
            return QVariant();

        if (index.row() < 0 || index.row() >= items.count())
            return QVariant();

        Syndication::ItemPtr item = items.at(index.row());
        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case 0: return item->title();
            case 1: return QLocale().toString(QDateTime::fromTime_t(item->datePublished()), QLocale::ShortFormat);
            case 2: return TorrentUrlFromItem(item);
            default:
                return QVariant();
            }
        }
        else if (role == Qt::DecorationRole && index.column() == 0 && feed->downloaded(item))
            return QIcon::fromTheme("go-down");

        return QVariant();
    }

    bool FeedWidgetModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        endRemoveRows();
        return true;
    }

    bool FeedWidgetModel::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    Syndication::ItemPtr FeedWidgetModel::itemForIndex(const QModelIndex& index)
    {
        if (index.row() < 0 || index.row() >= items.count())
            return Syndication::ItemPtr();

        return items.at(index.row());
    }

    QString TorrentUrlFromItem(Syndication::ItemPtr item)
    {
        QList<Syndication::EnclosurePtr> encs = item->enclosures();
        foreach (Syndication::EnclosurePtr e, encs)
        {
            if (e->type() == "application/x-bittorrent" || e->url().endsWith(".torrent"))
                return e->url();
        }

        // Search for magnets on item's link as some RSS feeds only use this item to post the magnet link.
        QString link = item->link();
        if (!link.isEmpty())
        {
            // Note that syndication library prepends the channel link to the item link by default, so
            // we need to extract the magnet from the string.
            int magnetStartIndex = link.indexOf("magnet:");
            if (magnetStartIndex >= 0)
                return link.right(link.size() - magnetStartIndex);
        }

        QMultiMap<QString, QDomElement> props = item->additionalProperties();
        QMultiMap<QString, QDomElement>::iterator itr = props.begin();
        while (itr != props.end())
        {
            const QDomElement& elem = itr.value();
            if (elem.nodeName() == "torrent")
            {
                QDomElement uri = elem.firstChildElement("magnetURI");
                if (!uri.isNull())
                    return uri.text();
            }
            itr++;
        }


        return QString();
    }

    void FeedWidgetModel::updated()
    {
        if (!feed)
            return;

        items.clear();
        Syndication::FeedPtr ptr = feed->feedData();
        if (ptr)
            items = ptr->items();

        reset();
    }
}
