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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <set>
#include <util/log.h>
#include "peeruploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "packetwriter.h"

namespace bt
{

	PeerUploader::PeerUploader(Peer* peer) : peer(peer)
	{
	}


	PeerUploader::~PeerUploader()
	{}

	void PeerUploader::addRequest(const Request & r)
	{
		requests.append(r);
	//	Out() << "Num requests : " << requests.count() << endl;
	}
	
	void PeerUploader::removeRequest(const Request & r)
	{
	//	Out() << "removeRequest " << r.getIndex() << " " << r.getOffset() << endl;
		requests.remove(r);
	}
	
	Uint32 PeerUploader::update(ChunkManager & cman,Uint32 opt_unchoked)
	{
		Uint32 uploaded = 0;	

		PacketWriter & pw = peer->getPacketWriter();
		uploaded += pw.update();
		
		// if we have choked the peer do not upload
		if (peer->areWeChoked())
			return uploaded;
		
		if (peer->isSnubbed() && !peer->areWeChoked() &&
			cman.chunksLeft() != 0 && peer->getID() != opt_unchoked)
			return uploaded;

		while (!requests.empty()/* && pw.getNumPacketsToWrite() == 0*/)
		{	
			Request r = requests.front();
			Chunk* c = cman.grabChunk(r.getIndex());
			
			if (c)
			{
				pw.sendChunk(r.getIndex(),r.getOffset(),r.getLength(),c);
				requests.pop_front();
				uploaded += pw.update();
			}
			else
			{
				// remove requests we can't satisfy
				Out() << "Cannot satisfy request" << endl;
				requests.pop_front();
			}
		}
		
		return uploaded;
	}
}
