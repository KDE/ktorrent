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

#ifndef KTIWFILELISTMODEL_H
#define KTIWFILELISTMODEL_H

#include <torrent/torrentfilelistmodel.h>

namespace kt
{
    /**
     *
     * @author Joris Guisson
     *
     * Expands the standard TorrentFileListModel to show more information.
    */
    class IWFileListModel : public TorrentFileListModel
    {
        Q_OBJECT
    public:
        IWFileListModel(bt::TorrentInterface* tc, QObject* parent);
        ~IWFileListModel();

        void changeTorrent(bt::TorrentInterface* tc) override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;
        void update() override;

        void filePercentageChanged(bt::TorrentFileInterface* file, float percentage) override;
        void filePreviewChanged(bt::TorrentFileInterface* file, bool preview) override;

    private:
        QVariant displayData(const QModelIndex& index) const;
        QVariant sortData(const QModelIndex& index) const;

    private:
        bool preview;
        bool mmfile;
        double percentage;
    };

}

#endif
