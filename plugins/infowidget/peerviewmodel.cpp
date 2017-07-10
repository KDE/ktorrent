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

#include "peerviewmodel.h"

#include <QIcon>
#include <QLocale>
#include <QStandardPaths>

#include <KLocalizedString>

#include <interfaces/torrentinterface.h>
#include <util/functions.h>
#include "flagdb.h"
#include "geoipmanager.h"

using namespace bt;

namespace kt
{
    static QIcon yes, no;
    static bool icons_loaded = false;
    static FlagDB flagDB(22, 18);


    PeerViewModel::Item::Item(bt::PeerInterface* peer, GeoIPManager* geo_ip) : peer(peer)
    {
        stats = peer->getStats();
        if (!icons_loaded)
        {
            yes = QIcon::fromTheme(QStringLiteral("dialog-ok"));
            no = QIcon::fromTheme(QStringLiteral("dialog-cancel"));
            icons_loaded = true;

            QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QStringLiteral("kf5/locale/countries"),
                                                  QStandardPaths::LocateDirectory);
            if (!path.isEmpty())
                flagDB.addFlagSource(path + QStringLiteral("/%1/flag.png"));
        }

        if (geo_ip)
        {
            int country_id = geo_ip->findCountry(stats.ip_address);
            if (country_id > 0)
            {
                country = geo_ip->countryName(country_id);
                flag = flagDB.getFlag(geo_ip->countryCode(country_id));
            }
        }
    }

    bool PeerViewModel::Item::changed() const
    {
        const PeerInterface::Stats& s = peer->getStats();
        bool ret =
            s.download_rate != stats.download_rate ||
            s.upload_rate != stats.upload_rate ||
            s.choked != stats.choked ||
            s.snubbed != stats.snubbed ||
            s.perc_of_file != stats.perc_of_file ||
            s.aca_score != stats.aca_score ||
            s.has_upload_slot != stats.has_upload_slot ||
            s.num_down_requests != stats.num_down_requests ||
            s.num_up_requests != stats.num_up_requests ||
            s.bytes_downloaded != stats.bytes_downloaded ||
            s.bytes_uploaded != stats.bytes_uploaded ||
            s.interested != stats.interested ||
            s.am_interested != stats.am_interested;
        stats = s;
        return ret;
    }

    QVariant PeerViewModel::Item::data(int col) const
    {
        switch (col)
        {
        case 0:
            if (stats.transport_protocol == bt::UTP)
                return QString(stats.address() + i18n(" (µTP)"));
            else
                return stats.address();
        case 1: return country;
        case 2: return stats.client;
        case 3:
            if (stats.download_rate >= 103)
                return BytesPerSecToString(stats.download_rate);
            else
                return QVariant();
        case 4:
            if (stats.upload_rate >= 103)
                return BytesPerSecToString(stats.upload_rate);
            else
                return QVariant();
        case 5: return stats.choked ? i18nc("Choked", "Yes") : i18nc("Not choked", "No");
        case 6: return stats.snubbed ? i18nc("Snubbed", "Yes") : i18nc("Not snubbed", "No");
        case 7: return QString(QString::number((int)stats.perc_of_file) + QLatin1String(" %"));
        case 8: return QVariant();
        case 9: return QLocale().toString(stats.aca_score, 'g', 2);
        case 10: return QVariant();
        case 11: return QString(QString::number(stats.num_down_requests) + QLatin1String(" / ") + QString::number(stats.num_up_requests));
        case 12: return BytesToString(stats.bytes_downloaded);
        case 13: return BytesToString(stats.bytes_uploaded);
        case 14: return stats.interested ? i18nc("Interested", "Yes") : i18nc("Not Interested", "No");
        case 15: return stats.am_interested ? i18nc("Interesting", "Yes") : i18nc("Not Interesting", "No");
        default: return QVariant();
        }
        return QVariant();
    }

    QVariant PeerViewModel::Item::sortData(int col) const
    {
        switch (col)
        {
        case 0: return stats.address();
        case 1: return country;
        case 2: return stats.client;
        case 3: return stats.download_rate;
        case 4: return stats.upload_rate;
        case 5: return stats.choked;
        case 6: return stats.snubbed;
        case 7: return stats.perc_of_file;
        case 8: return stats.dht_support;
        case 9: return stats.aca_score;
        case 10: return stats.has_upload_slot;
        case 11: return stats.num_down_requests + stats.num_up_requests;
        case 12: return stats.bytes_downloaded;
        case 13: return stats.bytes_uploaded;
        case 14: return stats.interested;
        case 15: return stats.am_interested;
        default: return QVariant();
        }
    }


    QVariant PeerViewModel::Item::decoration(int col) const
    {
        switch (col)
        {
        case 0:
            if (stats.encrypted)
                return QIcon::fromTheme(QStringLiteral("kt-encrypted"));
            break;
        case 1: return flag;
        case 8: return stats.dht_support ? yes : no;
        case 10: return stats.has_upload_slot ? yes : QIcon();
        }

        return QVariant();
    }

    /////////////////////////////////////////////////////////////

    PeerViewModel::PeerViewModel(QObject* parent)
        : QAbstractTableModel(parent), geo_ip(nullptr)
    {
        geo_ip = new GeoIPManager(this);
    }


    PeerViewModel::~PeerViewModel()
    {
        qDeleteAll(items);
    }

    void PeerViewModel::peerAdded(bt::PeerInterface* peer)
    {
        items.append(new Item(peer, geo_ip));
        insertRow(items.count() - 1);
    }

    void PeerViewModel::peerRemoved(bt::PeerInterface* peer)
    {
        for (QVector<Item*>::iterator i = items.begin(); i != items.end(); i++)
        {
            if ((*i)->peer == peer)
            {
                removeRow(i - items.begin());
                break;
            }
        }
    }

    void PeerViewModel::clear()
    {
        beginResetModel();
        qDeleteAll(items);
        items.clear();
        endResetModel();
    }

    void PeerViewModel::update()
    {
        int idx = 0;
        int lowest = -1;
        int highest = -1;

        foreach (Item* i, items)
        {
            if (i->changed())
            {
                if (lowest == -1)
                    lowest = idx;
                highest = idx;
            }
            idx++;
        }

        // emit only one data changed signal
        if (lowest != -1)
            emit dataChanged(index(lowest, 3), index(highest, 15));
    }

    QModelIndex PeerViewModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent) || parent.isValid())
            return QModelIndex();
        else
            return createIndex(row, column, items[row]);
    }

    int PeerViewModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return items.count();
    }

    int PeerViewModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 16;
    }

    QVariant PeerViewModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal)
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case 0: return i18n("Address");
            case 1: return i18n("Country");
            case 2: return i18n("Client");
            case 3: return i18n("Down Speed");
            case 4: return i18n("Up Speed");
            case 5: return i18n("Choked");
            case 6: return i18n("Snubbed");
            case 7: return i18n("Availability");
            case 8: return i18n("DHT");
            case 9: return i18n("Score");
            case 10: return i18n("Upload Slot");
            case 11: return i18n("Requests");
            case 12: return i18n("Downloaded");
            case 13: return i18n("Uploaded");
            case 14: return i18n("Interested");
            case 15: return i18n("Interesting");
            default: return QVariant();
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            switch (section)
            {
            case 0: return i18n("IP address of the peer");
            case 1: return i18n("Country the peer is in");
            case 2: return i18n("Which client the peer is using");
            case 3: return i18n("Download speed");
            case 4: return i18n("Upload speed");
            case 5: return i18n("Whether or not the peer has choked us - when we are choked the peer will not send us any data");
            case 6: return i18n("Snubbed means the peer has not sent us any data in the last 2 minutes");
            case 7: return i18n("How much data the peer has of the torrent");
            case 8: return i18n("Whether or not the peer has DHT enabled");
            case 9: return i18n("The score of the peer, KTorrent uses this to determine who to upload to");
            case 10: return i18n("Only peers which have an upload slot will get data from us");
            case 11: return i18n("The number of download and upload requests");
            case 12: return i18n("How much data we have downloaded from this peer");
            case 13: return i18n("How much data we have uploaded to this peer");
            case 14: return i18n("Whether the peer is interested in downloading data from us");
            case 15: return i18n("Whether we are interested in downloading from this peer");
            default: return QVariant();
            }
        }

        return QVariant();
    }

    QVariant PeerViewModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() >= items.count())
            return QVariant();

        Item* item = items[index.row()];
        if (role == Qt::DisplayRole)
            return item->data(index.column());
        else if (role == Qt::UserRole)
            return item->sortData(index.column());
        else if (role == Qt::DecorationRole)
            return item->decoration(index.column());

        return QVariant();
    }

    bool PeerViewModel::removeRows(int row, int count, const QModelIndex& /*parent*/)
    {
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++)
            delete items[row + i];
        items.remove(row, count);
        endRemoveRows();
        return true;
    }

    bool PeerViewModel::insertRows(int row, int count, const QModelIndex& /*parent*/)
    {
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bt::PeerInterface* PeerViewModel::indexToPeer(const QModelIndex& index)
    {
        if (!index.isValid() || index.row() >= items.count())
            return nullptr;
        else
            return ((Item*)index.internalPointer())->peer;
    }


}
