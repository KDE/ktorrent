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
#include <util/log.h>
#include "packetwriter.h"
#include "peer.h"
#include "request.h"
#include "chunk.h"
#include <util/bitset.h>
#include "packet.h"
#include "uploadcap.h"
#include <util/log.h>
#include "globals.h"

namespace bt
{

#ifdef DEBUG_LOG_UPLOAD
	static Log ulog;
	static bool upload_log_initialized = false;
#endif


	PacketWriter::PacketWriter(Peer* peer) : peer(peer)
	{
		uploaded = 0;
		packets.setAutoDelete(true);
#ifdef DEBUG_LOG_UPLOAD
		if (!upload_log_initialized)
		{
			ulog.setOutputFile("upload.log");
			upload_log_initialized = true;
		}
#endif
	}


	PacketWriter::~PacketWriter()
	{
		UploadCap::instance().killed(this);
	}

	void PacketWriter::sendPacket(const Packet & p)
	{
#ifdef DEBUG_LOG_UPLOAD
		ulog << p.debugString() << endl;
#endif
		peer->sendData(p.getHeader(),p.getHeaderLength());
		if (p.getDataLength() > 0)
			peer->sendData(p.getData(),p.getDataLength(),p.getType() == PIECE);
		if (p.getType() == PIECE)
			uploaded += p.getDataLength();
	}

	void PacketWriter::sendChoke()
	{
		if (peer->am_choked == true)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(CHOKE));
		else
			packets.append(new Packet(CHOKE));

		peer->am_choked = true;
	}
	
	void PacketWriter::sendUnchoke()
	{
		if (peer->am_choked == false)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(UNCHOKE));
		else
			packets.append(new Packet(UNCHOKE));
		peer->am_choked = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (peer->am_interested == true)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(INTERESTED));
		else
			packets.append(new Packet(INTERESTED));
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
		if (peer->am_interested == false)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(NOT_INTERESTED));
		else
			packets.append(new Packet(NOT_INTERESTED));
		peer->am_interested = false;
	}

	/*
	void PacketWriter::sendKeepAlive()
	{
		Uint8 buf[4];
		WriteUint32(buf,0,0);
		peer->sendData(buf,4);
	}*/
	
	void PacketWriter::sendRequest(const Request & r)
	{
		if (packets.count() == 0)
			sendPacket(Packet(r,false));
		else
			packets.append(new Packet(r,false));
	}
	
	void PacketWriter::sendCancel(const Request & r)
	{
		if (packets.count() == 0)
			sendPacket(Packet(r,true));
		else
			packets.append(new Packet(r,true));
	}
	
	void PacketWriter::sendHave(Uint32 index)
	{
		if (packets.count() == 0)
			sendPacket(Packet(index));
		else
			packets.append(new Packet(index));
	}
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		if (packets.count() == 0)
			sendPacket(Packet(bs));
		else
			packets.append(new Packet(bs));
	}
			
	void PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,const Chunk & ch)
	{
		if (begin >= ch.getSize() || begin + len > ch.getSize())
		{
			Out() << "Warning : Illegal piece request" << endl;
			Out() << "\tChunk : index " << index << " size = " << ch.getSize() << endl;
			Out() << "\tPiece : begin = " << begin << " len = " << len << endl;
		}
		else
		{
			// try to send it, if we can add it to the queue
			if (UploadCap::instance().allow(this))
			{
				sendPacket(Packet(index,begin,len,ch));
			//	Out() << "Sending " << index << " " << begin << endl;
			}
			else
				packets.append(new Packet(index,begin,len,ch));
		}
	}

	Uint32 PacketWriter::update()
	{
		Uint32 data_sent = uploaded;
		uploaded = 0;
		if (packets.count() == 0)
			return data_sent;
		
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			// send packets with no payload immediatly
			if (p->getType() != PIECE)
			{
				sendPacket(*p);
				packets.removeFirst();
			}
			else
			{
				break;
			}
		}

		return data_sent;
	}

	void PacketWriter::uploadUnsentPacket(bool all)
	{
		if (packets.count() == 0)
			return;
		
		// get rid of small packets first
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			// break if we have found a piece
			if (p->getType() == PIECE)
				break;
			sendPacket(*p);
			packets.removeFirst();
		}
		
		if (packets.count() == 0)
			return;
		
		if (!all)
		{
			// send a big one
			Packet* p = packets.first();	
		//	Out() << "Sending big one" << endl;
			sendPacket(*p);
			packets.removeFirst();
		}
		else
		{
			// send all packets left
			while (packets.count() > 0)
			{
				Packet* p = packets.first();
				if (p->getType() == PIECE)
				{
			//		Out() << "Sending big one" << endl;
				}
				sendPacket(*p);
				packets.removeFirst();
			}	
		}
	}
}
