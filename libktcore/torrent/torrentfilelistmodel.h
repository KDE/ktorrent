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

#ifndef KTTORRENTFILELISTMODEL_H
#define KTTORRENTFILELISTMODEL_H

#include "torrentfilemodel.h"

namespace kt
{

    /**
     * Model for displaying file trees of a torrent
     * @author Joris Guisson
    */
    class KTCORE_EXPORT TorrentFileListModel : public TorrentFileModel
    {
        Q_OBJECT
    public:
        TorrentFileListModel(bt::TorrentInterface* tc, DeselectMode mode, QObject* parent);
        ~TorrentFileListModel();

        void changeTorrent(bt::TorrentInterface* tc) override;
        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;
        void checkAll() override;
        void uncheckAll() override;
        void invertCheck() override;
        bt::Uint64 bytesToDownload() override;
        bt::TorrentFileInterface* indexToFile(const QModelIndex& idx) override;
        QString dirPath(const QModelIndex& idx) override;
        void changePriority(const QModelIndexList& indexes, bt::Priority newpriority) override;

    private:
        void invertCheck(const QModelIndex& idx);
    };

}

#endif
