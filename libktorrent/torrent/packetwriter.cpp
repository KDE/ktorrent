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
//#define DEBUG_LOG_UPLOAD
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

	bool PacketWriter::sendPacket(Packet & p,Uint32 max)
	{
#ifdef DEBUG_LOG_UPLOAD
		ulog << p.debugString() << endl;
#endif
		// safety check
		if (!p.isOK())
			return true;
			
		bool ret = true;
		Uint32 bs = 0;
	//	Out() << "Sending " << p.getHeaderLength() << " " << p.getDataLength() << endl;
		if (max == 0)
		{
			// send full packet
			ret = p.send(peer,p.getDataLength() + p.getHeaderLength(),bs);
		}
		else
		{
			ret = p.send(peer,max,bs);
		}
		
		if (p.getType() == PIECE)
		{
			peer->stats.bytes_uploaded += bs;
			uploaded += bs;
		}
		
		return ret;
	}
	
	void PacketWriter::queuePacket(Packet* p,bool ask)
	{
		bool ok = true;
		if (ask)
			ok = UploadCap::instance().allow(this,p->getDataLength() + p->getHeaderLength());
		
		
		if (ok && packets.count() == 0)
		{
			sendPacket(*p,0);
			delete p;
		}
		else 
		{
			packets.append(p);
		}
	}
	
	

	void PacketWriter::sendChoke()
	{
		if (peer->am_choked == true)
			return;
		
		queuePacket(new Packet(CHOKE),false);
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendUnchoke()
	{
//		Out() << "UNCHOKE" << endl;
		if (peer->am_choked == false)
			return;
		
		queuePacket(new Packet(UNCHOKE),false);
		peer->am_choked = false;
		peer->stats.has_upload_slot = true;
	}
	
	void PacketWriter::sendEvilUnchoke()
	{
		queuePacket(new Packet(UNCHOKE),false);
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (peer->am_interested == true)
			return;
		
		queuePacket(new Packet(INTERESTED),false);
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
		if (peer->am_interested == false)
			return;
		
		queuePacket(new Packet(NOT_INTERESTED),false);
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
		queuePacket(new Packet(r,false),false);
	}
	
	void PacketWriter::sendCancel(const Request & r)
	{
		queuePacket(new Packet(r,true),false);
	}
	
	void PacketWriter::sendHave(Uint32 index)
	{
		queuePacket(new Packet(index),false);
	}
	
	void PacketWriter::sendPort(Uint16 port)
	{
		queuePacket(new Packet(port),false);
	}
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		queuePacket(new Packet(bs),false);
	}
			
	void PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch)
	{
//		Out() << "sendChunk " << index << " " << begin << " " << len << endl;
		if (begin >= ch->getSize() || begin + len > ch->getSize())
		{
			Out() << "Warning : Illegal piece request" << endl;
			Out() << "\tChunk : index " << index << " size = " << ch->getSize() << endl;
			Out() << "\tPiece : begin = " << begin << " len = " << len << endl;
		}
		else
		{
			queuePacket(new Packet(index,begin,len,ch),true);
		}
	}

	Uint32 PacketWriter::update()
	{
		if (packets.count() == 0)
		{
			Uint32 data_sent = uploaded;
			uploaded = 0;
			return data_sent;
		}
		
		sendSmallPackets();
		
		// if there is a limit return data_sent
		if (UploadCap::instance().getMaxSpeed() > 0)
		{
			Uint32 data_sent = uploaded;
			uploaded = 0;
			return data_sent;
		}
		
		// no limit, go wild
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			sendPacket(*p,0);
			packets.removeFirst();
		}
		
		
		Uint32 data_sent = uploaded;
		uploaded = 0;
		return data_sent;
	}

	
	void PacketWriter::uploadUnsentBytes(Uint32 bytes)
	{
		if (packets.count() == 0)
			return;
		
		if (bytes == 0)
		{
			// send all packets left
			while (packets.count() > 0)
			{
				Packet* p = packets.first();
				if (sendPacket(*p,0))
					packets.removeFirst();
			}
		}
		else
		{
			Packet* p = packets.first();
			if (sendPacket(*p,bytes))
			{
				packets.removeFirst();
				sendSmallPackets();
			}
		}
	}
	
	void PacketWriter::sendSmallPackets()
	{
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			if (p->getType() == PIECE)
				return;
			
			sendPacket(*p,0);
			packets.removeFirst();
		}
	}
	
}
