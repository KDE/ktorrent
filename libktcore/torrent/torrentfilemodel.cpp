/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "torrentfilemodel.h"

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
TorrentFileModel::TorrentFileModel(bt::TorrentInterface *tc, DeselectMode mode, QObject *parent)
    : QAbstractItemModel(parent)
    , tc(tc)
    , mode(mode)
    , file_names_editable(false)
{
}

TorrentFileModel::~TorrentFileModel()
{
}

QByteArray TorrentFileModel::saveExpandedState(QSortFilterProxyModel *, QTreeView *)
{
    return QByteArray();
}

void TorrentFileModel::loadExpandedState(QSortFilterProxyModel *, QTreeView *, const QByteArray &)
{
}

void TorrentFileModel::missingFilesMarkedDND()
{
    beginResetModel();
    endResetModel();
}

void TorrentFileModel::update()
{
}

void TorrentFileModel::onCodecChange()
{
    beginResetModel();
    endResetModel();
}

Qt::ItemFlags TorrentFileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (tc->getStats().multi_file_torrent)
        flags |= Qt::ItemIsUserCheckable;

    if (fileNamesEditable() && index.column() == 0)
        flags |= Qt::ItemIsEditable;

    return flags;
}

void TorrentFileModel::filePercentageChanged(bt::TorrentFileInterface *file, float percentage)
{
    Q_UNUSED(file);
    Q_UNUSED(percentage);
}

void TorrentFileModel::filePreviewChanged(bt::TorrentFileInterface *file, bool preview)
{
    Q_UNUSED(file);
    Q_UNUSED(preview);
}
}
