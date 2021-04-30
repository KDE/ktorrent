/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    bt::TorrentInterface *tc;
    PeerView *pv;
    ChunkDownloadView *cdv;
    FileView *fv;

public:
    Monitor(bt::TorrentInterface *tc, PeerView *pv, ChunkDownloadView *cdv, FileView *fv);
    ~Monitor() override;

    void downloadRemoved(bt::ChunkDownloadInterface *cd) override;
    void downloadStarted(bt::ChunkDownloadInterface *cd) override;
    void peerAdded(bt::PeerInterface *peer) override;
    void peerRemoved(bt::PeerInterface *peer) override;
    void stopped() override;
    void destroyed() override;
    void filePercentageChanged(bt::TorrentFileInterface *file, float percentage) override;
    void filePreviewChanged(bt::TorrentFileInterface *file, bool preview) override;
};
}

#endif
