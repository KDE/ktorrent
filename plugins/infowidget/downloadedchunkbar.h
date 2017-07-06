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

#ifndef DOWNLOADEDCHUNKBAR_H
#define DOWNLOADEDCHUNKBAR_H

#include <torrent/chunkbar.h>
namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    /**
    @author Joris Guisson
    */
    class DownloadedChunkBar : public ChunkBar
    {
        Q_OBJECT
    public:
        DownloadedChunkBar(QWidget* parent);
        ~DownloadedChunkBar();

        const bt::BitSet& getBitSet() const override;
        void updateBar(bool force = false) override;
        void drawBarContents(QPainter* p) override;

        void setTC(bt::TorrentInterface* tc);
    private:
        bt::TorrentInterface* curr_tc;
        bt::BitSet curr_ebs;
    };
}

#endif
