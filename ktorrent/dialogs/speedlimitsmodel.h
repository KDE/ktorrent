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

#ifndef KTSPEEDLIMITSMODEL_H
#define KTSPEEDLIMITSMODEL_H

#include <QAbstractTableModel>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    class Core;

    /**
     * Model for the SpeedLimitsDlg main list view
    */
    class SpeedLimitsModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        SpeedLimitsModel(Core* core, QObject* parent);
        ~SpeedLimitsModel();

        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        void apply();

    signals:
        void enableApply(bool on);

    private:
        bt::TorrentInterface* torrentForIndex(const QModelIndex& index) const;

    private slots:
        void onTorrentAdded(bt::TorrentInterface* tc);
        void onTorrentRemoved(bt::TorrentInterface* tc);

    private:
        struct Limits
        {
            bt::Uint32 up;
            bt::Uint32 up_original;
            bt::Uint32 down;
            bt::Uint32 down_original;
            bt::Uint32 assured_up;
            bt::Uint32 assured_up_original;
            bt::Uint32 assured_down;
            bt::Uint32 assured_down_original;
        };

        Core* core;
        QMap<bt::TorrentInterface*, Limits> limits;
    };

}

#endif
