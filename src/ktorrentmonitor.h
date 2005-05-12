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
#ifndef KTORRENTMONITOR_H
#define KTORRENTMONITOR_H

#include <libtorrent/torrentmonitor.h>

class PeerView;
class ChunkDownloadView;

namespace bt
{
	class TorrentControl;
}

/**
@author Joris Guisson
*/
class KTorrentMonitor : public bt::TorrentMonitor
{
	bt::TorrentControl* tc;
	PeerView* pv;
	ChunkDownloadView* cdv;
public:
	KTorrentMonitor(
			bt::TorrentControl* tc,
			PeerView* pv,
			ChunkDownloadView* cdv);
	virtual ~KTorrentMonitor();

	virtual void downloadRemoved(bt::ChunkDownload* cd);
	virtual void downloadStarted(bt::ChunkDownload* cd);
	virtual void peerAdded(bt::Peer* peer);
	virtual void peerRemoved(bt::Peer* peer);
	virtual void stopped();
	virtual void destroyed();

};

#endif
