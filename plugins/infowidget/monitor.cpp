/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "monitor.h"

#include "chunkdownloadview.h"
#include "fileview.h"
#include "peerview.h"
#include <interfaces/chunkdownloadinterface.h>
#include <interfaces/peerinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>

using namespace bt;

namespace kt
{
Monitor::Monitor(bt::TorrentInterface *tc, PeerView *pv, ChunkDownloadView *cdv, FileView *fv)
    : tc(tc)
    , pv(pv)
    , cdv(cdv)
    , fv(fv)
{
    if (tc)
        tc->setMonitor(this);
}

Monitor::~Monitor()
{
    if (tc)
        tc->setMonitor(0);
}

void Monitor::downloadRemoved(bt::ChunkDownloadInterface *cd)
{
    if (cdv)
        cdv->downloadRemoved(cd);
}

void Monitor::downloadStarted(bt::ChunkDownloadInterface *cd)
{
    if (cdv)
        cdv->downloadAdded(cd);
}

void Monitor::peerAdded(bt::PeerInterface *peer)
{
    if (pv)
        pv->peerAdded(peer);
}

void Monitor::peerRemoved(bt::PeerInterface *peer)
{
    if (pv)
        pv->peerRemoved(peer);
}

void Monitor::stopped()
{
    if (pv)
        pv->removeAll();
    if (cdv)
        cdv->removeAll();
}

void Monitor::destroyed()
{
    if (pv)
        pv->removeAll();
    if (cdv)
        cdv->removeAll();
    tc = 0;
}

void Monitor::filePercentageChanged(bt::TorrentFileInterface *file, float percentage)
{
    if (fv)
        fv->filePercentageChanged(file, percentage);
}

void Monitor::filePreviewChanged(bt::TorrentFileInterface *file, bool preview)
{
    if (fv)
        fv->filePreviewChanged(file, preview);
}
}
