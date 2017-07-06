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

#ifndef KTDOWNLOADORDERMANAGER_H
#define KTDOWNLOADORDERMANAGER_H

#include <QList>
#include <QObject>
#include <util/constants.h>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{

    /**
        Class which manages the file download order for a single torrent
    */
    class DownloadOrderManager : public QObject
    {
        Q_OBJECT
    public:
        DownloadOrderManager(bt::TorrentInterface* tor);
        ~DownloadOrderManager();

        /// See if the file download order is enabled
        bool enabled() const {return order.count() > 0;}

        /// Save the order from torX/download_order
        void save();

        /// Load the order to torX/download_order
        void load();

        /// Enable the download order
        void enable();

        /// Disable the download order
        void disable();

        typedef QList<bt::Uint32> Order;

        /// Get the download order
        const Order& downloadOrder() const {return order;}

        /// Set the order
        void setDownloadOrder(const Order& norder) {order = norder;}

    public slots:
        /**
         * Change file priorities if needed
         */
        void update();

        /**
         * Change file priorities if needed
         */
        void chunkDownloaded(bt::TorrentInterface* me, bt::Uint32 chunk);

    private:
        bt::Uint32 nextIncompleteFile();

    private:
        bt::TorrentInterface* tor;
        QList<bt::Uint32> order;
        bt::Uint32 current_high_priority_file;
        bt::Uint32 current_normal_priority_file;
    };

}

#endif
