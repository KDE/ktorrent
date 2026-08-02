/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "peerviewmodel.h"

#include <QIcon>
#include <QLocale>
#include <QStandardPaths>

#include <KLocalizedString>

#include "flagdb.h"
#include <interfaces/torrentinterface.h>
#include <util/functions.h>

#if BUILD_WITH_GEOIP
#include "geoipmanager.h"
#endif

using namespace bt;

namespace kt
{
static QIcon yes, no;
static bool icons_loaded = false;
static FlagDB flagDB;

PeerViewModel::Item::Item(bt::PeerInterface *peer
#if BUILD_WITH_GEOIP
                          ,
                          GeoIPManager *geo_ip
#endif
                          )
    : peer(peer)
{
    stats = peer->getStats();
    if (!icons_loaded) {
        yes = QIcon::fromTheme(QStringLiteral("dialog-ok"));
        no = QIcon::fromTheme(QStringLiteral("dialog-cancel"));
        icons_loaded = true;
    }

#if BUILD_WITH_GEOIP
    const Country c = geo_ip->findCountry(stats.ip_address);
    country = c.name;
    flag = flagDB.getFlag(c.code);
#endif
}

bool PeerViewModel::Item::changed() const
{
    const PeerInterface::Stats &s = peer->getStats();
    // clang-format off
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
    // clang-format on
    stats = s;
    return ret;
}

QVariant PeerViewModel::Item::data(Column col) const
{
    switch (col) {
    case Column::ADDRESS:
        if (stats.transport_protocol == bt::UTP) {
            return QString(stats.address() + i18n(" (µTP)"));
        } else {
            return stats.address();
        }
    case Column::COUNTRY:
        return country;
    case Column::CLIENT:
        return stats.client;
    case Column::DOWNLOAD_SPEED:
        if (stats.download_rate >= 103) {
            return BytesPerSecToString(stats.download_rate);
        } else {
            return QVariant();
        }
    case Column::UPLOAD_SPEED:
        if (stats.upload_rate >= 103) {
            return BytesPerSecToString(stats.upload_rate);
        } else {
            return QVariant();
        }
    case Column::CHOKED:
        return stats.choked ? i18nc("Choked", "Yes") : i18nc("Not choked", "No");
    case Column::SNUBBED:
        return stats.snubbed ? i18nc("Snubbed", "Yes") : i18nc("Not snubbed", "No");
    case Column::AVAILABILITY:
        return i18nc("File percentage stat", "%1%", (int)stats.perc_of_file);
    case Column::DHT:
        return QVariant();
    case Column::SCORE:
        return QLocale().toString(stats.aca_score, 'f', 2);
    case Column::UPLOAD_SLOT:
        return QVariant();
    case Column::REQUESTS:
        return QString(QString::number(stats.num_down_requests) + QLatin1String(" / ") + QString::number(stats.num_up_requests));
    case Column::DOWNLOADED:
        return BytesToString(stats.bytes_downloaded);
    case Column::UPLOADED:
        return BytesToString(stats.bytes_uploaded);
    case Column::INTERESTED:
        return stats.interested ? i18nc("Interested", "Yes") : i18nc("Not Interested", "No");
    case Column::INTERESTING:
        return stats.am_interested ? i18nc("Interesting", "Yes") : i18nc("Not Interesting", "No");
    default:
        return QVariant();
    }
    return QVariant();
}

QVariant PeerViewModel::Item::sortData(Column col) const
{
    switch (col) {
    case Column::ADDRESS:
        return stats.address();
    case Column::COUNTRY:
        return country;
    case Column::CLIENT:
        return stats.client;
    case Column::DOWNLOAD_SPEED:
        return stats.download_rate;
    case Column::UPLOAD_SPEED:
        return stats.upload_rate;
    case Column::CHOKED:
        return stats.choked;
    case Column::SNUBBED:
        return stats.snubbed;
    case Column::AVAILABILITY:
        return stats.perc_of_file;
    case Column::DHT:
        return stats.dht_support;
    case Column::SCORE:
        return stats.aca_score;
    case Column::UPLOAD_SLOT:
        return stats.has_upload_slot;
    case Column::REQUESTS:
        return stats.num_down_requests + stats.num_up_requests;
    case Column::DOWNLOADED:
        return stats.bytes_downloaded;
    case Column::UPLOADED:
        return stats.bytes_uploaded;
    case Column::INTERESTED:
        return stats.interested;
    case Column::INTERESTING:
        return stats.am_interested;
    default:
        return QVariant();
    }
}

QVariant PeerViewModel::Item::decoration(Column col) const
{
    switch (col) {
    case Column::ADDRESS:
        if (stats.encrypted) {
            return QIcon::fromTheme(QStringLiteral("kt-encrypted"));
        }
        break;
    case Column::COUNTRY:
        return flag;
    case Column::DHT:
        return stats.dht_support ? yes : no;
    case Column::UPLOAD_SLOT:
        return stats.has_upload_slot ? yes : QIcon();
    default:
        break;
    }
    return QVariant();
}

/////////////////////////////////////////////////////////////

PeerViewModel::PeerViewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
#if BUILD_WITH_GEOIP
    geo_ip = new GeoIPManager(this);
#endif
}

PeerViewModel::~PeerViewModel()
{
    qDeleteAll(items);
}

void PeerViewModel::peerAdded(bt::PeerInterface *peer)
{
    items.append(new Item(peer
#if BUILD_WITH_GEOIP
                          ,
                          geo_ip
#endif
                          ));
    insertRow(items.count() - 1);
}

void PeerViewModel::peerRemoved(bt::PeerInterface *peer)
{
    int row = 0;
    bool found = false;
    for (Item *i : std::as_const(items)) {
        if (i->peer == peer) {
            found = true;
            break;
        }
        row++;
    }

    if (found) {
        removeRow(row);
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

    for (Item *i : std::as_const(items)) {
        if (i->changed()) {
            if (lowest == -1) {
                lowest = idx;
            }
            highest = idx;
        }
        idx++;
    }

    // emit only one data changed signal
    if (lowest != -1) {
        Q_EMIT dataChanged(index(lowest, 3), index(highest, 15));
    }
}

QModelIndex PeerViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid()) {
        return QModelIndex();
    } else {
        return createIndex(row, column, items[row]);
    }
}

int PeerViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return items.count();
    }
}

int PeerViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return NUM_COLUMNS;
    }
}

QVariant PeerViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    const Column column{section};
    if (role == Qt::DisplayRole) {
        switch (column) {
        case Column::ADDRESS:
            return i18n("Address");
        case Column::COUNTRY:
            return i18n("Country");
        case Column::CLIENT:
            return i18n("Client");
        case Column::DOWNLOAD_SPEED:
            return i18n("Down Speed");
        case Column::UPLOAD_SPEED:
            return i18n("Up Speed");
        case Column::CHOKED:
            return i18n("Choked");
        case Column::SNUBBED:
            return i18n("Snubbed");
        case Column::AVAILABILITY:
            return i18n("Availability");
        case Column::DHT:
            return i18n("DHT");
        case Column::SCORE:
            return i18n("Score");
        case Column::UPLOAD_SLOT:
            return i18n("Upload Slot");
        case Column::REQUESTS:
            return i18n("Requests");
        case Column::DOWNLOADED:
            return i18n("Downloaded");
        case Column::UPLOADED:
            return i18n("Uploaded");
        case Column::INTERESTED:
            return i18n("Interested");
        case Column::INTERESTING:
            return i18n("Interesting");
        default:
            return QVariant();
        }
    } else if (role == Qt::ToolTipRole) {
        switch (column) {
        case Column::ADDRESS:
            return i18n("IP address of the peer");
        case Column::COUNTRY:
            return i18n("Country the peer is in");
        case Column::CLIENT:
            return i18n("Which client the peer is using");
        case Column::DOWNLOAD_SPEED:
            return i18n("Download speed");
        case Column::UPLOAD_SPEED:
            return i18n("Upload speed");
        case Column::CHOKED:
            return i18n("Whether or not the peer has choked us - when we are choked the peer will not send us any data");
        case Column::SNUBBED:
            return i18n("Snubbed means the peer has not sent us any data in the last 2 minutes");
        case Column::AVAILABILITY:
            return i18n("How much data the peer has of the torrent");
        case Column::DHT:
            return i18n("Whether or not the peer has DHT enabled");
        case Column::SCORE:
            return i18n("The score of the peer, KTorrent uses this to determine who to upload to");
        case Column::UPLOAD_SLOT:
            return i18n("Only peers which have an upload slot will get data from us");
        case Column::REQUESTS:
            return i18n("The number of download and upload requests");
        case Column::DOWNLOADED:
            return i18n("How much data we have downloaded from this peer");
        case Column::UPLOADED:
            return i18n("How much data we have uploaded to this peer");
        case Column::INTERESTED:
            return i18n("Whether the peer is interested in downloading data from us");
        case Column::INTERESTING:
            return i18n("Whether we are interested in downloading from this peer");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant PeerViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= items.count()) {
        return QVariant();
    }

    const Column column{index.column()};
    Item *item = items[index.row()];
    if (role == Qt::DisplayRole) {
        return item->data(column);
    } else if (role == Qt::UserRole) {
        return item->sortData(column);
    } else if (role == Qt::DecorationRole) {
        return item->decoration(column);
    }

    return QVariant();
}

bool PeerViewModel::removeRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++) {
        delete items[row + i];
    }
    items.remove(row, count);
    endRemoveRows();
    return true;
}

bool PeerViewModel::insertRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bt::PeerInterface *PeerViewModel::indexToPeer(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= items.count()) {
        return nullptr;
    } else {
        return ((Item *)index.internalPointer())->peer;
    }
}

}

#include "moc_peerviewmodel.cpp"
