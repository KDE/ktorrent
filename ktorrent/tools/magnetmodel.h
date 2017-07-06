/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#ifndef KT_MAGNETMODEL_H
#define KT_MAGNETMODEL_H

#include <QAbstractTableModel>
#include <QPointer>
#include <util/constants.h>

namespace bt
{
    class MagnetDownloader;
}

namespace kt
{
   class MagnetManager;

   class MagnetModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        MagnetModel(MagnetManager *magnetManager, QObject* parent = 0);
        ~MagnetModel();

        /// Remove a magnet downloader
        void removeMagnets(int row, int count);

        /// Start a magnet downloader
        void start(int row, int count);

        /// Stop a magnet downloader
        void stop(int row, int count);

        /// Check if the magnet downloader that correspond to row is stopped
        bool isStopped(int row) const;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        bool removeRows(int row, int count, const QModelIndex& parent) override;
        bool insertRows(int row, int count, const QModelIndex& parent) override;

   public slots:
        void onUpdateQueue(bt::Uint32 idx, bt::Uint32 count);

    private:
        QString displayName(const bt::MagnetDownloader* md) const;
        QString status(int row) const;

    private:
        int currentRows;
        QPointer<MagnetManager> mman;
    };

}

#endif // KT_MAGNETMODEL_H
