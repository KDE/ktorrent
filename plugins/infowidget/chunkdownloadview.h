/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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

#ifndef KT_CHUNKDOWNLOADVIEW_HH
#define KT_CHUNKDOWNLOADVIEW_HH


#include <QSortFilterProxyModel>
#include <QTreeView>

#include <KSharedConfig>

#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/torrentinterface.h>
#include "ui_chunkdownloadview.h"


namespace kt
{
    class ChunkDownloadModel;


    /**
     * View which shows a list of downloading chunks, of a torrent.
     * */
    class ChunkDownloadView : public QWidget, public Ui_ChunkDownloadView
    {
        Q_OBJECT
    public:
        ChunkDownloadView(QWidget* parent);
        ~ChunkDownloadView();

        /// A peer has been added
        void downloadAdded(bt::ChunkDownloadInterface* cd);

        /// A download has been removed
        void downloadRemoved(bt::ChunkDownloadInterface* cd);

        /// Check to see if the GUI needs to be updated
        void update();

        /// Change the torrent to display
        void changeTC(bt::TorrentInterface* tc);

        /// Remove all items
        void removeAll();

        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

    private:
        bt::TorrentInterface::WPtr curr_tc;
        ChunkDownloadModel* model;
        QSortFilterProxyModel* pm;
    };
}

#endif
