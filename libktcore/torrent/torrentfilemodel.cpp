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

#include "torrentfilemodel.h"

#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>

namespace kt
{
    TorrentFileModel::TorrentFileModel(bt::TorrentInterface* tc, DeselectMode mode, QObject* parent)
        : QAbstractItemModel(parent), tc(tc), mode(mode), file_names_editable(false)
    {}

    TorrentFileModel::~TorrentFileModel()
    {}

    QByteArray TorrentFileModel::saveExpandedState(QSortFilterProxyModel*, QTreeView*)
    {
        return QByteArray();
    }

    void TorrentFileModel::loadExpandedState(QSortFilterProxyModel* , QTreeView* , const QByteArray&)
    {}

    void TorrentFileModel::missingFilesMarkedDND()
    {
        beginResetModel();
        endResetModel();
    }

    void TorrentFileModel::update()
    {}

    void TorrentFileModel::onCodecChange()
    {
        beginResetModel();
        endResetModel();
    }

    Qt::ItemFlags TorrentFileModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if (tc->getStats().multi_file_torrent)
            flags |= Qt::ItemIsUserCheckable;

        if (fileNamesEditable() && index.column() == 0)
            flags |= Qt::ItemIsEditable;

        return flags;
    }

    void TorrentFileModel::filePercentageChanged(bt::TorrentFileInterface* file, float percentage)
    {
        Q_UNUSED(file);
        Q_UNUSED(percentage);
    }

    void TorrentFileModel::filePreviewChanged(bt::TorrentFileInterface* file, bool preview)
    {
        Q_UNUSED(file);
        Q_UNUSED(preview);
    }
}

