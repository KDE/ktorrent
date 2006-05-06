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
#include <util/functions.h>
#include "peeruploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "packetwriter.h"

namespace bt
{

	PeerUploader::PeerUploader(Peer* peer) : peer(peer)
	{
		rone_time = 0;
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
		
		
		if (requests.count() > 1 || requests.count() == 0)
		{
			rone_time = bt::GetCurrentTime();
		}
		
		bool rone_send = (requests.count() == 1 && bt::GetCurrentTime() - rone_time > 5000);
		bool requests_left = requests.count() > 1 || rone_send;
		

		while ( requests_left && pw.getNumPacketsToWrite() == 0)
		{	
			Request r = requests.front();
			Chunk* c = cman.grabChunk(r.getIndex());

			if (c)
			{
				pw.sendChunk(r.getIndex(),r.getOffset(),r.getLength(),c);
				requests.pop_front();
				
			/*	if (peer->getStats().has_upload_slot)
				{
					Out() << QString("Peer : sending %1 %2 %3 : %4 requests left")
							.arg(r.getIndex())
							.arg(r.getOffset())
							.arg(r.getLength())
							.arg(requests.count()) << endl;
			}*/
				uploaded += pw.update();
			}
			else
			{
				// remove requests we can't satisfy
				Out() << "Cannot satisfy request" << endl;
				requests.pop_front();
			}
			
			if (rone_send)
				rone_time = bt::GetCurrentTime();
			
			rone_send = (requests.count() == 1 && bt::GetCurrentTime() - rone_time > 5000);
			requests_left = requests.count() > 1 || rone_send;
		}
		
		return uploaded;
	}
}
