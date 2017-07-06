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

#ifndef KTDOWNLOADORDERMODEL_H
#define KTDOWNLOADORDERMODEL_H

#include <QAbstractListModel>
#include <util/constants.h>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{

    /**
        Model for the download order in the dialog
    */
    class DownloadOrderModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        DownloadOrderModel(bt::TorrentInterface* tor, QObject* parent);
        ~DownloadOrderModel();

        /// Initialize the order
        void initOrder(const QList<bt::Uint32> & sl) {order = sl;}

        /// Get the order
        const QList<bt::Uint32> & downloadOrder() const {return order;}

        /// Find a text in the file list
        QModelIndex find(const QString& text);

        /// Clear high lights
        void clearHighLights();

        int rowCount(const QModelIndex& parent) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        Qt::DropActions supportedDropActions() const override;
        QStringList mimeTypes() const override;
        QMimeData* mimeData(const QModelIndexList& indexes) const override;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

        void moveUp(int row, int count);
        void moveDown(int row, int count);
        void moveTop(int row, int count);
        void moveBottom(int row, int count);

    public slots:
        void sortByName();
        void sortBySeasonsAndEpisodes();
        void sortByAlbumTrackOrder();

    private:
        bt::TorrentInterface* tor;
        QList<bt::Uint32> order;
        QString current_search_text;
    };

}

#endif
