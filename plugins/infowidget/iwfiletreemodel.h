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

#ifndef KTIWFILETREEMODEL_H
#define KTIWFILETREEMODEL_H

#include <torrent/torrentfiletreemodel.h>

namespace kt
{

    /**
     *
     * @author Joris Guisson
     *
     * Expands the standard TorrentFileTreeModel to show more information.
    */
    class IWFileTreeModel : public TorrentFileTreeModel
    {
        Q_OBJECT
    public:
        IWFileTreeModel(bt::TorrentInterface* tc, QObject* parent);
        ~IWFileTreeModel();

        void changeTorrent(bt::TorrentInterface* tc) override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        void update() override;
        void changePriority(const QModelIndexList& indexes, bt::Priority newpriority) override;

        void filePercentageChanged(bt::TorrentFileInterface* file, float percentage);
        void filePreviewChanged(bt::TorrentFileInterface* file, bool preview);

    private:
        void update(const QModelIndex& index, bt::TorrentFileInterface* file, int col);
        QVariant displayData(Node* n, const QModelIndex& index) const;
        QVariant sortData(Node* n, const QModelIndex& index) const;
        void setPriority(Node* n, bt::Priority newpriority, bool selected_node);

    private:
        bool preview;
        bool mmfile;
        double percentage;
    };

}

#endif
