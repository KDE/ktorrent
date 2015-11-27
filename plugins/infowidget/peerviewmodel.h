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
#ifndef KTPEERVIEWMODEL_H
#define KTPEERVIEWMODEL_H

#include <QIcon>
#include <QVector>
#include <QAbstractTableModel>
#include <interfaces/peerinterface.h>

namespace kt
{
    class GeoIPManager;

    /**
        @author Joris Guisson
        Model for the PeerView
    */
    class PeerViewModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        PeerViewModel(QObject* parent);
        virtual ~PeerViewModel();

        /// A peer has been added
        void peerAdded(bt::PeerInterface* peer);

        /// A peer has been removed
        void peerRemoved(bt::PeerInterface* peer);

        /**
         * Update the model
         */
        void update();

        /**
            Clear the model
        */
        void clear();

        virtual int rowCount(const QModelIndex& parent) const;
        virtual int columnCount(const QModelIndex& parent) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        virtual QVariant data(const QModelIndex& index, int role) const;
        virtual bool removeRows(int row, int count, const QModelIndex& parent);
        virtual bool insertRows(int row, int count, const QModelIndex& parent);
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

        bt::PeerInterface* indexToPeer(const QModelIndex& idx);

    public:
        struct Item
        {
            bt::PeerInterface* peer;
            mutable bt::PeerInterface::Stats stats;
            QString country;
            QIcon flag;

            Item(bt::PeerInterface* peer, GeoIPManager* geo_ip);

            bool changed() const;
            QVariant data(int col) const;
            QVariant decoration(int col) const;
            QVariant sortData(int col) const;
        };
    private:
        QVector<Item*> items;
        GeoIPManager* geo_ip;
    };

}

#endif
