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
//#define LOG_PACKET

#include <util/log.h>
#include <util/file.h>
#include <util/functions.h>
#include "packetwriter.h"
#include "peer.h"
#include "request.h"
#include "chunk.h"
#include <util/bitset.h>
#include "packet.h"
#include "uploadcap.h"
#include <util/log.h>
#include "globals.h"

#ifdef LOG_PACKET
#include <sys/types.h>
#include <unistd.h>
#endif

namespace bt
{
#ifdef LOG_PACKET
	static void LogPacket(const Uint8* data,Uint32 size)
	{
		QString file = QString("/tmp/kt-packetwriter-%1.log").arg(getpid());
		File fptr;
		if (!fptr.open(file,"a"))
			return;
			
		QString tmp = QString("PACKET len = %1, type = %2\nDATA: \n").arg(ReadUint32(data,0)).arg(data[4]);
		
		fptr.write(tmp.ascii(),tmp.length());
		
		data = data + 4;
		size -= 4;
		Uint32 j = 0;
		if (size <= 40)
		{
			for (Uint32 i = 0;i < size;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
		}
		else
		{
			for (Uint32 i = 0;i < 20;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
			tmp = QString("\n ... \n");
			fptr.write(tmp.ascii(),tmp.length());
			for (Uint32 i = size - 20;i < size;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
		}
		fptr.write("\n",1);
	}
#endif

	PacketWriter::PacketWriter(Peer* peer) : peer(peer)
	{
		uploaded = 0;
		packets.setAutoDelete(true);
		time_of_last_transmit = bt::GetCurrentTime();
	}


	PacketWriter::~PacketWriter()
	{
//		UploadCap::instance().killed(this);
	}

	bool PacketWriter::sendPacket(Packet & p)
	{
		// safety check
		if (!p.isOK())
			return true;
			
		bool ret = true;
		Uint32 bs = 0;
	//	Out() << "Sending " << p.getHeaderLength() << " " << p.getDataLength() << endl;
		ret = p.send(peer,bs);
		if (p.getType() == PIECE)
		{
			peer->stats.bytes_uploaded += bs;
			uploaded += bs;
		}
		
		time_of_last_transmit = bt::GetCurrentTime();
		
		return ret;
	}
	
	void PacketWriter::queuePacket(Packet* p)
	{
		bool ok = true;		
		if (packets.count() == 0)
		{
			if (sendPacket(*p))
			{
#ifdef LOG_PACKET
				LogPacket(p->getData(),p->getDataLength());
#endif
				delete p;
			}
			else
				packets.append(p); // can't send if fully so queue it up
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
		
		queuePacket(new Packet(CHOKE));
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendUnchoke()
	{
//		Out() << "UNCHOKE" << endl;
		if (peer->am_choked == false)
			return;
		
		queuePacket(new Packet(UNCHOKE));
		peer->am_choked = false;
		peer->stats.has_upload_slot = true;
	}
	
	void PacketWriter::sendEvilUnchoke()
	{
		queuePacket(new Packet(UNCHOKE));
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (peer->am_interested == true)
			return;
		
		queuePacket(new Packet(INTERESTED));
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
		if (peer->am_interested == false)
			return;
		
		queuePacket(new Packet(NOT_INTERESTED));
		peer->am_interested = false;
	}
	
	void PacketWriter::sendRequest(const Request & r)
	{
		queuePacket(new Packet(r,bt::REQUEST));
	}
	
	void PacketWriter::sendCancel(const Request & r)
	{
		queuePacket(new Packet(r,bt::CANCEL));
	}
	
	void PacketWriter::sendReject(const Request & r)
	{
		queuePacket(new Packet(r,bt::REJECT_REQUEST));
	}
	
	void PacketWriter::sendHave(Uint32 index)
	{
		queuePacket(new Packet(index,bt::HAVE));
	}
	
	void PacketWriter::sendPort(Uint16 port)
	{
		queuePacket(new Packet(port));
	}
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		queuePacket(new Packet(bs));
	}
	
	void PacketWriter::sendHaveAll()
	{
		queuePacket(new Packet(bt::HAVE_ALL));
	}

	void PacketWriter::sendHaveNone()
	{
		queuePacket(new Packet(bt::HAVE_NONE));
	}
	
	void PacketWriter::sendSuggestPiece(Uint32 index)
	{
		queuePacket(new Packet(index,bt::SUGGEST_PIECE));
	}
	
	void PacketWriter::sendAllowedFast(Uint32 index)
	{
		queuePacket(new Packet(index,bt::ALLOWED_FAST));
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
			queuePacket(new Packet(index,begin,len,ch));
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
		
		// no limit, go wild
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			if (sendPacket(*p))
			{
#ifdef LOG_PACKET
				LogPacket(p->getData(),p->getDataLength());
#endif
				packets.removeFirst();
			}
			else
				break; // if we can't send the full packet
		}
		
		
		Uint32 data_sent = uploaded;
		uploaded = 0;
		return data_sent;
	}

	
	
}
