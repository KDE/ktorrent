/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "magnetmodel.h"
#include <KLocalizedString>
#include <QFile>
#include <QIcon>
#include <QTextStream>

#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <magnet/magnetdownloader.h>
#include <torrent/magnetmanager.h>
#include <util/error.h>
#include <util/log.h>

namespace kt
{
MagnetModel::MagnetModel(MagnetManager *magnetManager, QObject *parent)
    : QAbstractTableModel(parent)
    , currentRows(0)
    , mman(magnetManager)
{
    connect(mman.data(), &MagnetManager::updateQueue, this, &MagnetModel::onUpdateQueue);
}

MagnetModel::~MagnetModel()
{
}

void MagnetModel::removeMagnets(int row, int count)
{
    mman->removeMagnets(row, count);
}

void MagnetModel::start(int row, int count)
{
    mman->start(row, count);
}

void MagnetModel::stop(int row, int count)
{
    mman->stop(row, count);
}

bool MagnetModel::isStopped(int row) const
{
    return mman->isStopped(row);
}

void MagnetModel::onUpdateQueue(bt::Uint32 idx, bt::Uint32 count)
{
    int rows = mman->count();
    if (currentRows < rows) // add new rows
        insertRows(idx, rows - currentRows, QModelIndex());
    else if (currentRows > rows) // delete rows
        removeRows(idx, currentRows - rows, QModelIndex());

    currentRows = rows;
    Q_EMIT dataChanged(index(idx, 0), index(count, columnCount(QModelIndex())));
}

QVariant MagnetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mman->count())
        return QVariant();

    const MagnetDownloader *md = mman->getMagnetDownloader(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return displayName(md);
        case 1:
            return status(index.row());
        case 2:
            return md->numPeers();
        default:
            return QVariant();
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0)
            return QIcon::fromTheme(QStringLiteral("kt-magnet"));
    } else if (role == Qt::ToolTipRole) {
        if (index.column() == 0)
            return md->magnetLink().toString();
    }

    return QVariant();
}

QVariant MagnetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("Magnet Link");
        case 1:
            return i18n("Status");
        case 2:
            return i18n("Peers");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

int MagnetModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 3;
}

int MagnetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !mman)
        return 0;
    else
        return mman->count();
}

bool MagnetModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bool MagnetModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

QString MagnetModel::displayName(const bt::MagnetDownloader *md) const
{
    if (md->magnetLink().displayName().isEmpty())
        return md->magnetLink().toString();
    else
        return md->magnetLink().displayName();
}

QString MagnetModel::status(int row) const
{
    switch (mman->status(row)) {
    case MagnetManager::DOWNLOADING:
        return i18n("Downloading");

    case MagnetManager::QUEUED:
        return i18n("Queued");

    case MagnetManager::STOPPED:
    default:
        return i18n("Stopped");
    }
}
}

#include "moc_magnetmodel.cpp"
