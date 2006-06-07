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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <interfaces/peerinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include "ktorrentmonitor.h"
#include "peerview.h"
#include "chunkdownloadview.h"

using namespace bt;

namespace kt
{
	
	KTorrentMonitor::KTorrentMonitor(kt::TorrentInterface* tc,
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
	
	
	void KTorrentMonitor::downloadRemoved(kt::ChunkDownloadInterface* cd)
	{
		if (cdv)
			cdv->removeDownload(cd);
	}
	
	void KTorrentMonitor::downloadStarted(kt::ChunkDownloadInterface* cd)
	{
		if (cdv)
			cdv->addDownload(cd);
	}
	
	void KTorrentMonitor::peerAdded(kt::PeerInterface* peer)
	{
		if (pv)
			pv->addPeer(peer);
	}
	
	void KTorrentMonitor::peerRemoved(kt::PeerInterface* peer)
	{
		if (pv)
			pv->removePeer(peer);
	}
	
	void KTorrentMonitor::stopped()
	{
		if (pv)
			pv->removeAll();
		if (cdv)
			cdv->removeAll();
	}
	
	void KTorrentMonitor::destroyed()
	{
		if (pv)
			pv->removeAll();
		if (cdv)
			cdv->removeAll();
		tc = 0;
	}
}	
