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
#include <libutil/log.h>
#include "peeruploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "packetwriter.h"

namespace bt
{

	PeerUploader::PeerUploader(Peer* peer,ChunkManager & cman) 
	: peer(peer),cman(cman)
	{
	}


	PeerUploader::~PeerUploader()
	{}

	void PeerUploader::addRequest(const Request & r)
	{
		requests.append(r);
	}
	
	void PeerUploader::removeRequest(const Request & r)
	{
		requests.remove(r);
	}
	
	Uint32 PeerUploader::update()
	{
		Uint32 uploaded = 0;
		std::set<Uint32> grabbed;

		PacketWriter & pw = peer->getPacketWriter();
		uploaded += pw.update();
		
		if (peer->isSnubbed() && !peer->isChoked() && cman.chunksLeft() != 0)
			return 0;

		while (!requests.empty() && pw.getNumPacketsToWrite() == 0)
		{	
			Request r = requests.front();
			Chunk* c = cman.grabChunk(r.getIndex());

			if (c)
			{
				if (grabbed.count(r.getIndex()) == 0)
				{
					grabbed.insert(r.getIndex());
					c->ref();
				}
				pw.sendChunk(r.getIndex(),r.getOffset(),r.getLength(),*c);			
				requests.remove(r);
				uploaded += pw.update();
			}
		}
		
		std::set<Uint32>::iterator g = grabbed.begin();
		while (g != grabbed.end())
		{
			cman.releaseChunk(*g);
			g++;
		}
		return uploaded;
	}
}
