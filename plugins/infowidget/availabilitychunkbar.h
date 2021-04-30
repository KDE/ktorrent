/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AVAILABILITYCHUNKBAR_H
#define AVAILABILITYCHUNKBAR_H

#include <interfaces/torrentinterface.h>
#include <torrent/chunkbar.h>

namespace kt
{
/**
@author Joris Guisson
*/
class AvailabilityChunkBar : public ChunkBar
{
    Q_OBJECT
public:
    AvailabilityChunkBar(QWidget *parent);
    ~AvailabilityChunkBar() override;

    const bt::BitSet &getBitSet() const override;

    void setTC(bt::TorrentInterface *tc);

private:
    bt::TorrentInterface *curr_tc;
};
}

#endif
