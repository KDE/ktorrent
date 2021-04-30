/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_VIDEOCHUNKBAR_H
#define KT_VIDEOCHUNKBAR_H

#include "mediafile.h"
#include <torrent/chunkbar.h>

namespace kt
{
/**
    ChunkBar for a video during streaming mode
 */
class VideoChunkBar : public ChunkBar
{
    Q_OBJECT
public:
    VideoChunkBar(const MediaFileRef &mfile, QWidget *parent);
    ~VideoChunkBar() override;

    /// Set the media file
    void setMediaFile(const MediaFileRef &mf);

    /// Get the bitset
    const bt::BitSet &getBitSet() const override;

    /// Time has elapsed during playing, update the bar if necessary
    void timeElapsed(qint64 time);

private Q_SLOTS:
    void updateChunkBar();
    void updateBitSet();

private:
    void drawBarContents(QPainter *p) override;

private:
    MediaFileRef mfile;
    bt::BitSet bitset;
    bt::Uint32 current_chunk;
};

}

#endif // KT_VIDEOCHUNKBAR_H
