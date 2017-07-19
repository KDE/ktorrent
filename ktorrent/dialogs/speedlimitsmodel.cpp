/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include <KLocalizedString>

#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <util/functions.h>
#include "core.h"
#include "speedlimitsmodel.h"

using namespace bt;

namespace kt
{
    SpeedLimitsModel::SpeedLimitsModel(Core* core, QObject* parent) : QAbstractTableModel(parent), core(core)
    {
        kt::QueueManager* qman = core->getQueueManager();
        QList<bt::TorrentInterface*>::iterator itr = qman->begin();
        while (itr != qman->end())
        {
            Limits lim;
            bt::TorrentInterface* tc = *itr;
            tc->getTrafficLimits(lim.up_original, lim.down_original);
            lim.down = lim.down_original;
            lim.up = lim.up_original;
            tc->getAssuredSpeeds(lim.assured_up_original, lim.assured_down_original);
            lim.assured_down = lim.assured_down_original;
            lim.assured_up = lim.assured_up_original;
            limits.insert(tc, lim);
            itr++;
        }

        connect(core, &Core::torrentAdded, this, &SpeedLimitsModel::onTorrentAdded);
        connect(core, &Core::torrentRemoved, this, &SpeedLimitsModel::onTorrentRemoved);
    }

    SpeedLimitsModel::~SpeedLimitsModel()
    {}

    void SpeedLimitsModel::onTorrentAdded(bt::TorrentInterface* tc)
    {
        Limits lim;
        tc->getTrafficLimits(lim.up_original, lim.down_original);
        lim.down = lim.down_original;
        lim.up = lim.up_original;
        limits.insert(tc, lim);
        tc->getAssuredSpeeds(lim.assured_up_original, lim.assured_down_original);
        lim.assured_down = lim.assured_down_original;
        lim.assured_up = lim.assured_up_original;
        insertRow(limits.count() - 1);
    }

    void SpeedLimitsModel::onTorrentRemoved(bt::TorrentInterface* tc)
    {
        kt::QueueManager* qman = core->getQueueManager();
        int idx = 0;
        QList<bt::TorrentInterface*>::iterator itr = qman->begin();
        while (itr != qman->end())
        {
            if (*itr == tc)
                break;
            idx++;
            itr++;
        }

        limits.remove(tc);
        removeRow(idx);
    }

    int SpeedLimitsModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return core->getQueueManager()->count();
    }

    int SpeedLimitsModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 5;
    }

    QVariant SpeedLimitsModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        switch (section)
        {
        case 0: return i18n("Torrent");
        case 1: return i18n("Download Limit");
        case 2: return i18n("Upload Limit");
        case 3: return i18n("Assured Download Speed");
        case 4: return i18n("Assured Upload Speed");
        default:
            return QVariant();
        }
    }

    QVariant SpeedLimitsModel::data(const QModelIndex& index, int role) const
    {
        if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole)
            return QVariant();

        bt::TorrentInterface* tc = torrentForIndex(index);
        if (!tc)
            return QVariant();

        const Limits& lim = limits[tc];

        switch (index.column())
        {
        case 0: return tc->getDisplayName();
        case 1:
            if (role == Qt::EditRole || role == Qt::UserRole)
                return lim.down / 1024;
            else
                return lim.down == 0 ? i18n("No limit") : BytesPerSecToString(lim.down);
        case 2:
            if (role == Qt::EditRole || role == Qt::UserRole)
                return lim.up / 1024;
            else
                return lim.up == 0 ? i18n("No limit") : BytesPerSecToString(lim.up);
        case 3:
            if (role == Qt::EditRole || role == Qt::UserRole)
                return lim.assured_down / 1024;
            else
                return lim.assured_down == 0 ? i18n("No assured speed") : BytesPerSecToString(lim.assured_down);
        case 4:
            if (role == Qt::EditRole || role == Qt::UserRole)
                return lim.assured_up / 1024;
            else
                return lim.assured_up == 0 ? i18n("No assured speed") : BytesPerSecToString(lim.assured_up);
        default: return QVariant();
        }
    }

    bool SpeedLimitsModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::EditRole)
            return false;

        bt::TorrentInterface* tc = torrentForIndex(index);
        if (!tc || !limits.contains(tc))
            return false;

        bool ok = false;
        Limits& lim = limits[tc];

        switch (index.column())
        {
        case 1:
            lim.down = value.toInt(&ok) * 1024;
            break;
        case 2:
            lim.up = value.toInt(&ok) * 1024;
            break;
        case 3:
            lim.assured_down = value.toInt(&ok) * 1024;
            break;
        case 4:
            lim.assured_up = value.toInt(&ok) * 1024;
            break;
        }

        if (ok)
        {
            emit dataChanged(index, index);
            if (lim.up != lim.up_original || lim.down != lim.down_original ||
                    lim.assured_down != lim.assured_down_original || lim.up_original != lim.assured_up_original)
            {
                enableApply(true);
            }
        }

        return ok;
    }

    Qt::ItemFlags SpeedLimitsModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        if (index.column() > 0)
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        else
            return QAbstractItemModel::flags(index);
    }

    bt::TorrentInterface* SpeedLimitsModel::torrentForIndex(const QModelIndex& index) const
    {
        kt::QueueManager* qman = core->getQueueManager();
        int r = index.row();
        QList<bt::TorrentInterface*>::iterator itr = qman->begin();
        itr += r;

        if (itr == qman->end())
            return 0;
        else
            return *itr;
    }

    void SpeedLimitsModel::apply()
    {
        QMap<bt::TorrentInterface*, Limits>::iterator itr = limits.begin();
        while (itr != limits.end())
        {
            bt::TorrentInterface* tc = itr.key();
            Limits& lim = itr.value();
            if (lim.up != lim.up_original || lim.down != lim.down_original)
            {
                tc->setTrafficLimits(lim.up, lim.down);
                lim.up_original = lim.up;
                lim.down_original = lim.down;
            }

            if (lim.assured_up != lim.assured_up_original || lim.assured_down != lim.assured_down_original)
            {
                tc->setAssuredSpeeds(lim.assured_up, lim.assured_down);
                lim.assured_up_original = lim.assured_up;
                lim.assured_down_original = lim.assured_down;
            }
            itr++;
        }
    }
}

