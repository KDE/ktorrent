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
#include <set>
#include <ksocketaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <util/sha1hash.h>
#include "peeruploader.h"
#include "peer.h"
#include "chunkmanager.h"
#include "packetwriter.h"
#include "torrent.h"

using namespace KNetwork;

namespace bt
{

	PeerUploader::PeerUploader(Peer* peer) : peer(peer)
	{
		uploaded = 0;
	}


	PeerUploader::~PeerUploader()
	{}

	void PeerUploader::addRequest(const Request & r)
	{
	//	Out(SYS_CON|LOG_DEBUG) << 
	//			QString("PeerUploader::addRequest %1 %2 %3\n").arg(r.getIndex()).arg(r.getOffset()).arg(r.getLength()) << endl;
		
		// allowed fast chunks go to the front of the queue
		requests.append(r);
	}
	
	void PeerUploader::removeRequest(const Request & r)
	{
	//	Out(SYS_CON|LOG_DEBUG) << 
	//			QString("PeerUploader::removeRequest %1 %2 %3\n").arg(r.getIndex()).arg(r.getOffset()).arg(r.getLength()) << endl;
		requests.remove(r);
		peer->getPacketWriter().doNotSendPiece(r,peer->getStats().fast_extensions);
	}
	
	Uint32 PeerUploader::update(ChunkManager & cman,Uint32 opt_unchoked)
	{
		Uint32 ret = uploaded;
		uploaded = 0;
	
		PacketWriter & pw = peer->getPacketWriter();
		
		// if we have choked the peer do not upload
		if (peer->areWeChoked())
			return ret;
				
		if (peer->isSnubbed() && !peer->areWeChoked() &&
			!cman.completed() && peer->getID() != opt_unchoked)
			return ret;
		
		
		while (requests.count() > 0)
		{	
			Request r = requests.front();
			
			Chunk* c = cman.grabChunk(r.getIndex());	
			if (c && c->getData())
			{
				if (!pw.sendChunk(r.getIndex(),r.getOffset(),r.getLength(),c))
				{
					if (peer->getStats().fast_extensions)
						pw.sendReject(r);
				}
				requests.pop_front();
			}
			else
			{
				// remove requests we can't satisfy
				Out(SYS_CON|LOG_DEBUG) << "Cannot satisfy request" << endl;
				if (peer->getStats().fast_extensions)
					pw.sendReject(r);
				requests.pop_front();
			}
		}
		
		return ret;
	}
	
	void PeerUploader::clearAllRequests()
	{
		bool fast_ext = peer->getStats().fast_extensions;
		PacketWriter & pw = peer->getPacketWriter();
		pw.clearPieces(fast_ext);
		
		if (fast_ext)
		{
			// reject all requests 
			// if the peer supports fast extensions, 
			// choke doesn't mean reject all
			QValueList<Request>::iterator i = requests.begin();
			while (i != requests.end())
			{	
				pw.sendReject(*i);
				i++;
			}
		}
		requests.clear();
	}
		
	Uint32 PeerUploader::getNumRequests() const
	{
		return requests.count() + peer->getPacketWriter().getNumDataPacketsToWrite();
	}
}
