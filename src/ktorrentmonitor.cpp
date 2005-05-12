/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <libtorrent/peer.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/chunkdownload.h>
#include "ktorrentmonitor.h"
#include "peerview.h"
#include "chunkdownloadview.h"

using namespace bt;

KTorrentMonitor::KTorrentMonitor(bt::TorrentControl* tc,
			PeerView* pv,
			ChunkDownloadView* cdv) : tc(tc),pv(pv),cdv(cdv)
{
	if (tc)
		tc->setMonitor(this);
}


KTorrentMonitor::~KTorrentMonitor()
{
	if (tc)
		tc->setMonitor(0);
}


void KTorrentMonitor::downloadRemoved(ChunkDownload* cd)
{
	cdv->removeDownload(cd);
}

void KTorrentMonitor::downloadStarted(ChunkDownload* cd)
{
	cdv->addDownload(cd);
}

void KTorrentMonitor::peerAdded(Peer* peer)
{
	pv->addPeer(peer);
}

void KTorrentMonitor::peerRemoved(Peer* peer)
{
	pv->removePeer(peer);
}

void KTorrentMonitor::stopped()
{
	pv->removeAll();
	cdv->removeAll();
}

void KTorrentMonitor::destroyed()
{
	pv->removeAll();
	cdv->removeAll();
	tc = 0;
}

