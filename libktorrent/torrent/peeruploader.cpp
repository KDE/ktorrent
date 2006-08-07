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
		rone_time = 0;
		can_generate_af = false;
		uploaded = 0;
	}


	PeerUploader::~PeerUploader()
	{}

	void PeerUploader::addRequest(const Request & r)
	{
	//	Out(SYS_CON|LOG_DEBUG) << 
	//			QString("PeerUploader::addRequest %1 %2 %3\n").arg(r.getIndex()).arg(r.getOffset()).arg(r.getLength()) << endl;
		
		// allowed fast chunks go to the front of the queue
		if (allowed_fast.count(r.getIndex()) > 0)
			requests.prepend(r);
		else
			requests.append(r);
	}
	
	void PeerUploader::removeRequest(const Request & r)
	{
	//	Out(SYS_CON|LOG_DEBUG) << 
	//			QString("PeerUploader::removeRequest %1 %2 %3\n").arg(r.getIndex()).arg(r.getOffset()).arg(r.getLength()) << endl;
		requests.remove(r);
	}
	
	Uint32 PeerUploader::update(ChunkManager & cman,Uint32 opt_unchoked)
	{
		Uint32 ret = uploaded;
		uploaded = 0;
		// generate allowed fast set
		if (can_generate_af)
		{
			generateAF(cman);
			can_generate_af = false;
		}
		
		PacketWriter & pw = peer->getPacketWriter();
		
		// if we have choked the peer do not upload
		if (peer->areWeChoked() && allowed_fast.size() == 0)
			return ret;
				
		if (peer->isSnubbed() && !peer->areWeChoked() &&
			cman.chunksLeft() != 0 && peer->getID() != opt_unchoked)
			return ret;
		
		if (requests.count() > 1 || requests.count() == 0)
			rone_time = bt::GetCurrentTime();
		
		bool rone_send = (requests.count() == 1 && bt::GetCurrentTime() - rone_time > 5000);
		bool requests_left = requests.count() > 1 || rone_send;
		
		while (requests_left && pw.getNumPacketsToWrite() < 5)
		{	
			Request r = requests.front();
			// if we are choked only send when the request
			// is in the allowed_fast set
			if (peer->areWeChoked() && allowed_fast.count(r.getIndex()) == 0)
				return ret;

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
					
			if (rone_send)
				rone_time = bt::GetCurrentTime();
			
			rone_send = (requests.count() == 1 && bt::GetCurrentTime() - rone_time > 5000);
			requests_left = requests.count() > 1 || rone_send;
		}
		
		return ret;
	}
	
	void PeerUploader::rejectAll()
	{
		PacketWriter & pw = peer->getPacketWriter();
		while (requests.count() > 0)
		{
			Request r = requests.front();
			pw.sendReject(r);
			requests.pop_front();
		}
	}
	
	void PeerUploader::enableAllowedFast()
	{
		can_generate_af = true;
	}
	
	void PeerUploader::generateAF(ChunkManager & cman)
	{
		Uint32 cnt = 0;
		Uint8 tmp[24];
		SHA1Hash hash;
		KIpAddress addr(peer->getIPAddresss());
		Uint32 ip = addr.IPv4Addr();
		if (addr.isClassA() || addr.isClassB())
			bt::WriteUint32(tmp,0,ip & 0xFFFF0000);
		else
			bt::WriteUint32(tmp,0,ip & 0xFFFFFF00);
		
		memcpy(tmp+4,cman.getTorrent().getInfoHash().getData(),20);
		hash = SHA1Hash::generate(tmp,24);
		while (allowed_fast.size() < ALLOWED_FAST_SIZE && cnt < 10)
		{
			for (Uint32 i = 0;i < 5 && allowed_fast.size() < ALLOWED_FAST_SIZE;i++)
			{
				Uint32 y = bt::ReadUint32(hash.getData(),i*4);
				allowed_fast.insert(y % cman.getNumChunks());
			}
			hash = SHA1Hash::generate(hash.getData(),20);
			cnt++; // update counter so we stop after 10 attempts
		}
		
		PacketWriter & pw = peer->getPacketWriter();
		// send allowed fast set
		std::set<Uint32>::iterator itr = allowed_fast.begin();
		while (itr != allowed_fast.end())
		{
			// send allowed fast chunks
			pw.sendAllowedFast(*itr);
			itr++;
		}
	}
}
