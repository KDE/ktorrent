/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef KT_VIDEOCHUNKBAR_H
#define KT_VIDEOCHUNKBAR_H

#include <torrent/chunkbar.h>
#include "mediafile.h"

namespace kt
{

    /**
        ChunkBar for a video during streaming mode
     */
    class VideoChunkBar : public ChunkBar
    {
        Q_OBJECT
    public:
        VideoChunkBar(const MediaFileRef& mfile, QWidget* parent);
        ~VideoChunkBar();

        /// Set the media file
        void setMediaFile(const MediaFileRef& mf);

        /// Get the bitset
        const bt::BitSet& getBitSet() const override;

        /// Time has elapsed during playing, update the bar if necessary
        void timeElapsed(qint64 time);

    private slots:
        void updateChunkBar();
        void updateBitSet();

    private:
        void drawBarContents(QPainter* p) override;

    private:
        MediaFileRef mfile;
        bt::BitSet bitset;
        bt::Uint32 current_chunk;
    };

}

#endif // KT_VIDEOCHUNKBAR_H
