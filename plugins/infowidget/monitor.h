/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#ifndef KTMONITOR_H
#define KTMONITOR_H

#include <interfaces/monitorinterface.h>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    class PeerView;
    class ChunkDownloadView;
    class FileView;

    /**
    @author Joris Guisson
    */
    class Monitor : public bt::MonitorInterface
    {
        bt::TorrentInterface* tc;
        PeerView* pv;
        ChunkDownloadView* cdv;
        FileView* fv;
    public:
        Monitor(bt::TorrentInterface* tc, PeerView* pv , ChunkDownloadView* cdv, FileView* fv);
        ~Monitor();

        void downloadRemoved(bt::ChunkDownloadInterface* cd) override;
        void downloadStarted(bt::ChunkDownloadInterface* cd) override;
        void peerAdded(bt::PeerInterface* peer) override;
        void peerRemoved(bt::PeerInterface* peer) override;
        void stopped() override;
        void destroyed() override;
        void filePercentageChanged(bt::TorrentFileInterface* file, float percentage) override;
        void filePreviewChanged(bt::TorrentFileInterface* file, bool preview) override;
    };
}

#endif
