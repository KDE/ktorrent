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
#include <net/socketmonitor.h>
#include <ktversion.h>
#include "packetwriter.h"
#include "peer.h"
#include "request.h"
#include "chunk.h"
#include <util/bitset.h>
#include "packet.h"
#include "uploadcap.h"
#include <util/log.h>
#include "globals.h"
#include "bencoder.h"



namespace bt
{


	PacketWriter::PacketWriter(Peer* peer) : peer(peer),mutex(true) // this is a recursive mutex
	{
		uploaded = 0;
		uploaded_non_data = 0;
		curr_packet = 0;
		ctrl_packets_sent = 0;
	}


	PacketWriter::~PacketWriter()
	{
		std::list<Packet*>::iterator i = data_packets.begin();
		while (i != data_packets.end())
		{
			Packet* p = *i;
			delete p;
			i++;
		}
		
		i = control_packets.begin();
		while (i != control_packets.end())
		{
			Packet* p = *i;
			delete p;
			i++;
		}
	}
	
	void PacketWriter::queuePacket(Packet* p)
	{
		QMutexLocker locker(&mutex);
		if (p->getType() == PIECE)
			data_packets.push_back(p);
		else
			control_packets.push_back(p);
		// tell upload thread we have data ready should it be sleeping
		net::SocketMonitor::instance().signalPacketReady();
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
			
	bool PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch)
	{
//		Out() << "sendChunk " << index << " " << begin << " " << len << endl;
		if (begin >= ch->getSize() || begin + len > ch->getSize())
		{
			Out(SYS_CON|LOG_NOTICE) << "Warning : Illegal piece request" << endl;
			Out(SYS_CON|LOG_NOTICE) << "\tChunk : index " << index << " size = " << ch->getSize() << endl;
			Out(SYS_CON|LOG_NOTICE) << "\tPiece : begin = " << begin << " len = " << len << endl;
			return false;
		}
		else if (!ch || ch->getData() == 0)
		{
			Out(SYS_CON|LOG_NOTICE) << "Warning : attempted to upload an invalid chunk" << endl;
			return false;
		}
		else
		{
	/*		Out(SYS_CON|LOG_DEBUG) << QString("Uploading %1 %2 %3 %4 %5")
					.arg(index).arg(begin).arg(len).arg((Q_ULLONG)ch,0,16).arg((Q_ULLONG)ch->getData(),0,16) 
					<< endl;;
	*/
			queuePacket(new Packet(index,begin,len,ch));
			return true;
		}
	}
	
	void PacketWriter::sendExtProtHandshake(Uint16 port,bool pex_on)
	{
		QByteArray arr;
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		enc.write("m"); 
		// supported messages
		enc.beginDict();
		enc.write("ut_pex");enc.write((Uint32)(pex_on ? 1 : 0));
		enc.end();
		if (port > 0)
		{
			enc.write("p"); 
			enc.write((Uint32)port);
		}
		enc.write("v"); enc.write(QString("KTorrent %1").arg(kt::VERSION_STRING));
		enc.end();
		sendExtProtMsg(0,arr);
	}
	
	void PacketWriter::sendExtProtMsg(Uint8 id,const QByteArray & data)
	{
		queuePacket(new Packet(id,data));
	}
	
	Packet* PacketWriter::selectPacket()
	{
		Packet* ret = 0;
		// this function should ensure that between
		// each data packet at least 3 control packets are sent
		// so requests can get through
		
		if (ctrl_packets_sent < 3)
		{
			// try to send another control packet
			if (control_packets.size() > 0)
				ret = control_packets.front();
			else if (data_packets.size() > 0)
				ret = data_packets.front(); 
		}
		else
		{
			if (data_packets.size() > 0)
			{
				ctrl_packets_sent = 0;
				ret = data_packets.front();
			}
			else if (control_packets.size() > 0)
				ret = control_packets.front();
		}

		return ret;
	}
	
	Uint32 PacketWriter::onReadyToWrite(Uint8* data,Uint32 max_to_write)
	{
		QMutexLocker locker(&mutex);
		
		if (!curr_packet)
			curr_packet = selectPacket();
		
		Uint32 written = 0;
		while (curr_packet && written < max_to_write)
		{
			Packet* p = curr_packet;
			bool count_as_data = false;
			Uint32 ret = p->putInOutputBuffer(data + written,max_to_write - written,count_as_data);
			written += ret;
			if (count_as_data)
				uploaded += ret;
			else
				uploaded_non_data += ret;
					
			if (p->isSent())
			{
				// packet sent, so remove it
				if (p->getType() == PIECE)
				{
					// remove data packet
					data_packets.pop_front();
					delete p;
					// reset ctrl_packets_sent so the next packet should be a ctrl packet
					ctrl_packets_sent = 0;  
					curr_packet = selectPacket();
				}
				else
				{
					// remove control packet and select another one to send
					control_packets.pop_front();
					delete p;
					ctrl_packets_sent++;
					curr_packet = selectPacket();
				}
			}
			else
			{
				// we can't send it fully, so break out of loop
				break;
			}
		}
	
		return written;
	}
	
	bool PacketWriter::hasBytesToWrite() const
	{
		return getNumPacketsToWrite() > 0;
	}
	
	Uint32 PacketWriter::getUploadedDataBytes() const
	{
		QMutexLocker locker(&mutex);
		Uint32 ret = uploaded;
		uploaded = 0;
		return ret;
	}
	
	Uint32 PacketWriter::getUploadedNonDataBytes() const
	{
		QMutexLocker locker(&mutex);
		Uint32 ret = uploaded_non_data;
		uploaded_non_data = 0;
		return ret;
	}
	
	Uint32 PacketWriter::getNumPacketsToWrite() const
	{
		QMutexLocker locker(&mutex);
		return data_packets.size() + control_packets.size();
	}
	
	Uint32 PacketWriter::getNumDataPacketsToWrite() const
	{
		QMutexLocker locker(&mutex);
		return data_packets.size();
	}
	
	void PacketWriter::doNotSendPiece(const Request & req,bool reject)
	{
		QMutexLocker locker(&mutex);
		std::list<Packet*>::iterator i = data_packets.begin();
		while (i != data_packets.end())
		{
			Packet* p = *i;
			if (p->isPiece(req) && !p->sending())
			{
				// remove current item
				if (curr_packet == p)
					curr_packet = 0;
				
				i = data_packets.erase(i);
				if (reject)
				{
					// queue a reject packet
					sendReject(req);
				}
				delete p;
			}
			else
			{
				i++;
			}
		}
	}
	
	void PacketWriter::clearPieces(bool reject)
	{
		QMutexLocker locker(&mutex);
		
		std::list<Packet*>::iterator i = data_packets.begin();
		while (i != data_packets.end())
		{
			Packet* p = *i;
			if (p->getType() == bt::PIECE && !p->sending())
			{
				// remove current item
				if (curr_packet == p)
					curr_packet = 0;
				
				if (reject)
				{
					queuePacket(p->makeRejectOfPiece());
				}
					
				i = data_packets.erase(i);
				delete p;
			}
			else
			{
				i++;
			}
		}
	}
}
