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

#include "trackermodel.h"

#include <QColor>
#include <QList>

#include <KLocalizedString>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerinterface.h>

namespace kt
{

    TrackerModel::TrackerModel(QObject* parent)
        : QAbstractTableModel(parent), tc(nullptr), running(false)
    {
    }

    TrackerModel::~TrackerModel()
    {
        qDeleteAll(trackers);
    }

    void TrackerModel::changeTC(bt::TorrentInterface* tc)
    {
        beginResetModel();
        qDeleteAll(trackers);
        trackers.clear();
        this->tc = tc;
        if (tc)
        {
            QList<bt::TrackerInterface*> tracker_list = tc->getTrackersList()->getTrackers();
            for (bt::TrackerInterface* trk : qAsConst(tracker_list))
            {
                trackers.append(new Item(trk));
            }
        }
        endResetModel();
    }

    void TrackerModel::update()
    {
        if (!tc)
            return;

        int idx = 0;
        foreach (Item* t, trackers)
        {
            if (t->update())
                emit dataChanged(index(idx, 1), index(idx, 5));
            idx++;
        }

        running = tc->getStats().running;
    }


    int TrackerModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid() || !tc)
            return 0;
        else
            return trackers.count();
    }

    int TrackerModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 6;
    }


    QVariant TrackerModel::data(const QModelIndex& index, int role) const
    {
        if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
            return QVariant();

        Item* item = (Item*)index.internalPointer();
        if (!item)
            return QVariant();

        bt::TrackerInterface* trk = item->trk;

        if (role == Qt::CheckStateRole && index.column() == 0)
        {
            return trk->isEnabled() ? Qt::Checked : Qt::Unchecked;
        }
        else if (role == Qt::DisplayRole)
        {
            return item->displayData(index.column());
        }
        else if (role == Qt::UserRole)
        {
            return item->sortData(index.column());
        }
        else if (role == Qt::ForegroundRole && index.column() == 1 && trk->trackerStatus() == bt::TRACKER_ERROR)
        {
            return QColor(Qt::red);
        }

        return QVariant();
    }

    bool TrackerModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!tc || !index.isValid() || index.row() < 0 || index.row() >= trackers.count())
            return false;

        if (role == Qt::CheckStateRole)
        {
            QUrl url = trackers.at(index.row())->trk->trackerURL();
            tc->getTrackersList()->setTrackerEnabled(url, (Qt::CheckState)value.toUInt() == Qt::Checked);
            return true;
        }
        return false;
    }

    QVariant TrackerModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal)
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case 0: return i18n("URL");
            case 1: return i18n("Status");
            case 2: return i18n("Seeders");
            case 3: return i18n("Leechers");
            case 4: return i18n("Times Downloaded");
            case 5: return i18n("Next Update");
            }
        }
        return QVariant();
    }

    void TrackerModel::addTrackers(QList<bt::TrackerInterface*> & tracker_list)
    {
        if (tracker_list.isEmpty())
            return;

        int row = trackers.count();
        foreach (bt::TrackerInterface* trk, tracker_list)
            trackers.append(new Item(trk));

        insertRows(row, tracker_list.count(), QModelIndex());
    }

    bool TrackerModel::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool TrackerModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        if (tc)
        {
            for (int i = 0; i < count; i++)
            {
                Item* item = trackers.takeAt(row);
                QUrl url = item->trk->trackerURL();
                tc->getTrackersList()->removeTracker(url);
                delete item;
            }
        }
        endRemoveRows();
        return true;
    }

    Qt::ItemFlags TrackerModel::flags(const QModelIndex& index) const
    {
        if (!tc || !index.isValid() || index.row() >= trackers.count() || index.row() < 0 || index.column() != 0)
            return QAbstractItemModel::flags(index);
        else
            return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    QModelIndex TrackerModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (parent.isValid() || row < 0 || row >= trackers.count() || column < 0 || column >= 6)
            return QModelIndex();
        else
            return createIndex(row, column, trackers.at(row));
    }


    QUrl TrackerModel::trackerUrl(const QModelIndex& index)
    {
        if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
            return QUrl();

        return ((Item*)index.internalPointer())->trk->trackerURL();
    }

    bt::TrackerInterface* TrackerModel::tracker(const QModelIndex& index)
    {
        if (!tc || !index.isValid() ||  index.row() < 0 || index.row() >= trackers.count())
            return 0;

        return ((Item*)index.internalPointer())->trk;
    }

    //////////////////////////////////////////

    TrackerModel::Item::Item(bt::TrackerInterface* tracker)
        : trk(tracker)
        , status(tracker->trackerStatus())
        , seeders(-1)
        , leechers(-1)
        , times_downloaded(-1)
        , time_to_next_update(0)
    {
    }

    bool TrackerModel::Item::update()
    {
        bool ret = false;
        if (status != trk->trackerStatus())
        {
            status = trk->trackerStatus();
            ret = true;
        }

        if (seeders != trk->getNumSeeders())
        {
            seeders = trk->getNumSeeders();
            ret = true;
        }

        if (leechers != trk->getNumLeechers())
        {
            leechers = trk->getNumLeechers();
            ret = true;
        }

        if (times_downloaded != trk->getTotalTimesDownloaded())
        {
            times_downloaded = trk->getTotalTimesDownloaded();
            ret = true;
        }

        if (time_to_next_update != trk->timeToNextUpdate())
        {
            time_to_next_update = trk->timeToNextUpdate();
            ret = true;
        }

        return ret;
    }

    QVariant TrackerModel::Item::displayData(int column) const
    {
        switch (column)
        {
        case 0: return trk->trackerURL().toString();
        case 1: return trk->trackerStatusString();
        case 2: return seeders >= 0 ? seeders : QVariant();
        case 3: return leechers >= 0 ? leechers : QVariant();
        case 4: return times_downloaded >= 0 ? times_downloaded : QVariant();
        case 5:
        {
            int secs = time_to_next_update;
            if (secs)
                return QTime(0,0,0,0).addSecs(secs).toString(QStringLiteral("mm:ss"));
            else
                return QVariant();
        }
        default: return QVariant();
        }
    }

    QVariant TrackerModel::Item::sortData(int column) const
    {
        switch (column)
        {
        case 0: return trk->trackerURL().toString();
        case 1: return status;
        case 2: return seeders;
        case 3: return leechers;
        case 4: return times_downloaded;
        case 5: return time_to_next_update;
        default: return QVariant();
        }
    }

}
