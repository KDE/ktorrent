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

#ifndef KTCHUNKDOWNLOADMODEL_H
#define KTCHUNKDOWNLOADMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/torrentinterface.h>


namespace kt
{

    /**
        @author
    */
    class ChunkDownloadModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        ChunkDownloadModel(QObject* parent);
        ~ChunkDownloadModel();

        /// A peer has been added
        void downloadAdded(bt::ChunkDownloadInterface* cd);

        /// A download has been removed
        void downloadRemoved(bt::ChunkDownloadInterface* cd);

        /// change the current torrent
        void changeTC(bt::TorrentInterface* tc);

        /**
         * Update the model
         */
        void update();

        /**
            Clear the model
        */
        void clear();

        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        bool removeRows(int row, int count, const QModelIndex& parent) override;
        bool insertRows(int row, int count, const QModelIndex& parent) override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    public:
        struct Item
        {
            mutable bt::ChunkDownloadInterface::Stats stats;
            bt::ChunkDownloadInterface* cd;
            QString files;

            Item(bt::ChunkDownloadInterface* cd, const QString& files);

            bool changed() const;
            QVariant data(int col) const;
            QVariant sortData(int col) const;
        };
    private:
        QVector<Item*> items;
        bt::TorrentInterface::WPtr tc;
    };

}

#endif
