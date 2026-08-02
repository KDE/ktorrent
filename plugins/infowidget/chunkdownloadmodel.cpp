/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chunkdownloadmodel.h"

#include <KLocalizedString>

#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
ChunkDownloadModel::Item::Item(ChunkDownloadInterface *cd, const QString &files)
    : cd(cd)
    , files(files)
{
    cd->getStats(stats);
}

bool ChunkDownloadModel::Item::changed() const
{
    ChunkDownloadInterface::Stats s;
    cd->getStats(s);
    bool ret = s.pieces_downloaded != stats.pieces_downloaded || s.download_speed != stats.download_speed || s.current_peer_id != stats.current_peer_id;

    stats = s;
    return ret;
}

QVariant ChunkDownloadModel::Item::data(Column col) const
{
    switch (col) {
    case Column::CHUNK:
        return stats.chunk_index;
    case Column::PROGRESS:
        return QStringLiteral("%1 / %2").arg(stats.pieces_downloaded).arg(stats.total_pieces);
    case Column::PEER:
        return stats.current_peer_id;
    case Column::DOWNLOAD_SPEED:
        return BytesPerSecToString(stats.download_speed);
    case Column::FILES:
        return files;
    default:
        return QVariant();
    }
}

QVariant ChunkDownloadModel::Item::sortData(Column col) const
{
    switch (col) {
    case Column::CHUNK:
        return stats.chunk_index;
    case Column::PROGRESS:
        return stats.pieces_downloaded;
    case Column::PEER:
        return stats.current_peer_id;
    case Column::DOWNLOAD_SPEED:
        return stats.download_speed;
    case Column::FILES:
        return files;
    default:
        return QVariant();
    }
}

/////////////////////////////////////////////////////////////

ChunkDownloadModel::ChunkDownloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ChunkDownloadModel::~ChunkDownloadModel()
{
    qDeleteAll(items);
}

void ChunkDownloadModel::downloadAdded(bt::ChunkDownloadInterface *cd)
{
    if (!tc) {
        return;
    }

    bt::ChunkDownloadInterface::Stats stats;
    cd->getStats(stats);
    QString files;
    int n = 0;
    if (tc.data()->getStats().multi_file_torrent) {
        for (Uint32 i = 0; i < tc.data()->getNumFiles(); i++) {
            const bt::TorrentFileInterface &tf = tc.data()->getTorrentFile(i);
            if (stats.chunk_index >= tf.getFirstChunk() && stats.chunk_index <= tf.getLastChunk()) {
                if (n > 0) {
                    files += QStringLiteral(", ");
                }

                files += tf.getUserModifiedPath();
                n++;
            } else if (stats.chunk_index < tf.getFirstChunk()) {
                break;
            }
        }
    }

    Item *nitem = new Item(cd, files);
    items.append(nitem);
    insertRow(items.count() - 1);
}

void ChunkDownloadModel::downloadRemoved(bt::ChunkDownloadInterface *cd)
{
    int row = 0;
    bool found = false;
    for (Item *item : std::as_const(items)) {
        if (item->cd == cd) {
            found = true;
            break;
        }
        row++;
    }

    if (found) {
        removeRow(row);
    }
}

void ChunkDownloadModel::changeTC(bt::TorrentInterface *tc)
{
    beginResetModel();
    qDeleteAll(items);
    items.clear();
    endResetModel();
    this->tc = tc;
}

void ChunkDownloadModel::clear()
{
    beginResetModel();
    qDeleteAll(items);
    items.clear();
    endResetModel();
}

void ChunkDownloadModel::update()
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
        Q_EMIT dataChanged(index(lowest, 1), index(highest, 3));
    }
}

int ChunkDownloadModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return items.count();
    }
}

int ChunkDownloadModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return NUM_COLUMNS;
    }
}

QVariant ChunkDownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    const Column column{section};
    if (role == Qt::DisplayRole) {
        switch (column) {
        case Column::CHUNK:
            return i18n("Chunk");
        case Column::PROGRESS:
            return i18n("Progress");
        case Column::PEER:
            return i18n("Peer");
        case Column::DOWNLOAD_SPEED:
            return i18n("Down Speed");
        case Column::FILES:
            return i18n("Files");
        default:
            return QVariant();
        }
    } else if (role == Qt::ToolTipRole) {
        switch (column) {
        case Column::CHUNK:
            return i18n("Number of the chunk");
        case Column::PROGRESS:
            return i18n("Download progress of the chunk");
        case Column::PEER:
            return i18n("Which peer we are downloading it from");
        case Column::DOWNLOAD_SPEED:
            return i18n("Download speed of the chunk");
        case Column::FILES:
            return i18n("Which files the chunk is located in");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QModelIndex ChunkDownloadModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid()) {
        return QModelIndex();
    } else {
        return createIndex(row, column, items[row]);
    }
}

QVariant ChunkDownloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= items.count() || index.row() < 0) {
        return QVariant();
    }

    const Column column{index.column()};
    if (role == Qt::DisplayRole) {
        return items[index.row()]->data(column);
    } else if (role == Qt::UserRole) {
        return items[index.row()]->sortData(column);
    }

    return QVariant();
}

bool ChunkDownloadModel::removeRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++) {
        delete items[row + i];
    }
    items.remove(row, count);
    endRemoveRows();
    return true;
}

bool ChunkDownloadModel::insertRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}
}

#include "moc_chunkdownloadmodel.cpp"
