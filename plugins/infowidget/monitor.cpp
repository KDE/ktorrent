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

#include "monitor.h"

#include <interfaces/peerinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include "peerview.h"
#include "chunkdownloadview.h"
#include "fileview.h"

using namespace bt;

namespace kt
{

    Monitor::Monitor(bt::TorrentInterface* tc, PeerView* pv, ChunkDownloadView* cdv, FileView* fv)
        : tc(tc), pv(pv), cdv(cdv), fv(fv)
    {
        if (tc)
            tc->setMonitor(this);
    }


    Monitor::~Monitor()
    {
        if (tc)
            tc->setMonitor(0);
    }


    void Monitor::downloadRemoved(bt::ChunkDownloadInterface* cd)
    {
        if (cdv)
            cdv->downloadRemoved(cd);
    }

    void Monitor::downloadStarted(bt::ChunkDownloadInterface* cd)
    {
        if (cdv)
            cdv->downloadAdded(cd);
    }

    void Monitor::peerAdded(bt::PeerInterface* peer)
    {
        if (pv)
            pv->peerAdded(peer);
    }

    void Monitor::peerRemoved(bt::PeerInterface* peer)
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

    void Monitor::filePercentageChanged(bt::TorrentFileInterface* file, float percentage)
    {
        if (fv)
            fv->filePercentageChanged(file, percentage);
    }

    void Monitor::filePreviewChanged(bt::TorrentFileInterface* file, bool preview)
    {
        if (fv)
            fv->filePreviewChanged(file, preview);
    }
}
