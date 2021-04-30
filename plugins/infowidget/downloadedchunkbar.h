/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    DownloadedChunkBar(QWidget *parent);
    ~DownloadedChunkBar() override;

    const bt::BitSet &getBitSet() const override;
    void updateBar(bool force = false) override;
    void drawBarContents(QPainter *p) override;

    void setTC(bt::TorrentInterface *tc);

private:
    bt::TorrentInterface *curr_tc;
    bt::BitSet curr_ebs;
};
}

#endif
