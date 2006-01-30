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

	Uint32 PacketWriter::sendPacket(const Packet & p,Uint32 max)
	{
#ifdef DEBUG_LOG_UPLOAD
		ulog << p.debugString() << endl;
#endif
		// safety check
		if (!p.isOK())
			return p.getDataLength();
			
	//	Out() << "Sending " << p.getHeaderLength() << " " << p.getDataLength() << endl;
		if (max == 0)
		{
			peer->sendData(p.getHeader(),p.getHeaderLength());
			if (p.getDataLength() > 0)
				peer->sendData(p.getData(),p.getDataLength(),p.getType() == PIECE);
			if (p.getType() == PIECE)
				uploaded += p.getDataLength();
			return p.getDataLength();
		}
		else
		{
			// send header if no data of packet is sent
			if (p.getDataWritten() == 0)
				peer->sendData(p.getHeader(),p.getHeaderLength());
			
			Uint32 off = p.getDataWritten();
			Uint32 bytes_left = p.getDataLength() - off;
			Uint32 to_send = max > bytes_left ? bytes_left : max;
			peer->sendData(p.getData() + off,to_send,p.getType() == PIECE); 
			if (p.getType() == PIECE)
				uploaded += to_send;
			
			return to_send;
		}
	}
	
	void PacketWriter::queuePacket(Packet* p,bool ask)
	{
		bool ok = true;
		if (ask)
			ok = UploadCap::instance().allow(this,p->getDataLength());
		
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
	}
	
	void PacketWriter::sendUnchoke()
	{
		if (peer->am_choked == false)
			return;
		
		queuePacket(new Packet(UNCHOKE),false);
		peer->am_choked = false;
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
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		queuePacket(new Packet(bs),false);
	}
			
	void PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch)
	{
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
		Uint32 data_sent = uploaded;
		uploaded = 0;
		if (packets.count() == 0)
			return data_sent;
		
		sendSmallPackets();
		
		// if there is a limit return data_sent
		if (UploadCap::instance().getMaxSpeed() > 0)
			return data_sent;
		
		// no limit, go wild
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			sendPacket(*p,0);
			packets.removeFirst();
		}
		
		return data_sent;
	}

	
	Uint32 PacketWriter::uploadUnsentBytes(Uint32 bytes)
	{
		if (packets.count() == 0)
			return 0;
		
		if (bytes == 0)
		{
			// send all packets left
			while (packets.count() > 0)
			{
				Packet* p = packets.first();
				sendPacket(*p,0);
				packets.removeFirst();
			}
			return 0;
		}
		else
		{
			sendSmallPackets();
			Packet* p = packets.first();
			Uint32 nb = sendPacket(*p,bytes);
			p->dataWritten(nb);
			// if the packet is fully sent remove it
			if (p->getDataWritten() == p->getDataLength())
			{
				packets.removeFirst();
				sendSmallPackets();
			}
			return nb;
		}
	}
	
	void PacketWriter::sendSmallPackets()
	{
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			if (p->getType() == PIECE)
				break;
			
			sendPacket(*p,0);
			packets.removeFirst();
		}
	}
}
