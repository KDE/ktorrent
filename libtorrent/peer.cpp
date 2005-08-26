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
#include <libutil/log.h>
#include "peer.h"
#include "chunk.h"
#include "speedestimater.h"
#include "piece.h"
#include "request.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "peerdownloader.h"
#include "peeruploader.h"
#include <libutil/functions.h>

namespace bt
{
	
	
	static Uint32 peer_id_counter = 1;
	
	Peer::Peer(QSocket* sock,const PeerID & peer_id,Uint32 num_chunks) 
	: sock(sock),pieces(num_chunks),peer_id(peer_id)
	{
		id = peer_id_counter;
		peer_id_counter++;
		
		speed = new SpeedEstimater();
		preader = new PacketReader(sock,speed);
		choked = am_choked = true;
		interested = am_interested = false;
		killed = false;
		downloader = new PeerDownloader(this);
		uploader = new PeerUploader(this);
		
		connect(sock,SIGNAL(connectionClosed()),this,SLOT(connectionClosed()));
		connect(sock,SIGNAL(readyRead()),this,SLOT(readyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(error(int)));
		pwriter = new PacketWriter(this);
		time_choked = GetCurrentTime();
	}


	Peer::~Peer()
	{
		delete uploader;
		delete downloader;
		delete pwriter;
		delete preader;
		if (sock)
		{
			sock->close();
			delete sock;
		}
		delete speed;
	}

	void Peer::connectionClosed() 
	{
		Out() << "Connection Closed" << endl;
		closeConnection();
		killed = true;
	} 
	
	void Peer::closeConnection()
	{
		if (sock)
		{
			sock->close();
			delete sock;
			sock = 0;
		}
	}
	
	void Peer::readyRead() 
	{
		if (killed) return;
		readPacket();
	}
	
	void Peer::error(int)
	{
		//Out() << "Error : " << err << endl;
		closeConnection();
		killed = true;
	}

	void Peer::kill()
	{
		closeConnection();
		killed = true;
	}

	
	void Peer::readPacket()
	{
		if (killed) return;
		
		while (preader->readPacket() && preader->ok())
		{
			handlePacket(preader->getPacketLength());
		}
		
		if (!preader->ok())
			error(0);
	}
	
	void Peer::handlePacket(Uint32 len)
	{
		if (killed) return;
		
		if (len == 0)
			return;
		const Uint8* tmp_buf = preader->getData();
		//Out() << "Got packet : " << len << " type = " << type <<  endl;
		Uint8 type = tmp_buf[0];
		switch (type)
		{
			case CHOKE:
				if (len != 1)
				{
					Out() << "len err CHOKE" << endl;
					error(0);
					return;
				}
				
				if (!choked)
				{
					time_choked = GetCurrentTime();
				}
				choked = true;
				break;
			case UNCHOKE:
				if (len != 1)
				{
					Out() << "len err UNCHOKE" << endl;
					error(0);
					return;
				}
				
				choked = false;
				break;
			case INTERESTED:
				if (len != 1)
				{
					Out() << "len err INTERESTED" << endl;
					error(0);
					return;
				}
				
				interested = true;
				break;
			case NOT_INTERESTED:
				if (len != 1)
				{
					Out() << "len err NOT_INTERESTED" << endl;
					error(0);
					return;
				}
				
				interested = false;
				break;
			case HAVE:
				if (len != 5)
				{
					Out() << "len err HAVE" << endl;
					error(0);
					return;
				}
				
				haveChunk(this,ReadUint32(tmp_buf,1));
				pieces.set(ReadUint32(tmp_buf,1),true);
				break;
			case BITFIELD:
				if (len != 1 + pieces.getNumBytes())
				{
					Out() << "len err BITFIELD" << endl;
					error(0);
					return;
				}
				
				pieces = BitSet(tmp_buf+1,pieces.getNumBits());
				break;
			case REQUEST:
				if (len != 13)
				{
					Out() << "len err REQUEST" << endl;
					error(0);
					return;
				}
				
				{
					Request r(
							ReadUint32(tmp_buf,1),
							ReadUint32(tmp_buf,5),
							ReadUint32(tmp_buf,9),
							id);
					
					uploader->addRequest(r);
					Out() << "REQUEST " << peer_id.toString() << endl;
				}
				break;
			case PIECE:
				if (len < 9)
				{
					Out() << "len err PIECE" << endl;
					error(0);
					return;
				}
				
				snub_timer.update();
				
				{
					Piece p(ReadUint32(tmp_buf,1),
							ReadUint32(tmp_buf,5),
							len - 9,id,tmp_buf+9);
					piece(p);
				}
				break;
			case CANCEL:
				if (len != 13)
				{
					Out() << "len err CANCEL" << endl;
					error(0);
					return;
				}
				
				{
					Request r(ReadUint32(tmp_buf,1),
							  ReadUint32(tmp_buf,5),
							  ReadUint32(tmp_buf,9),
							  id);
					uploader->removeRequest(r);
				}
				break;
		}
	}
	
	void Peer::sendData(const Uint8* data,Uint32 len,bool record)
	{
		if (killed) return;
		
		sock->writeBlock((const char*)data,len);
		if (record)
			speed->onWrite(len);
	}
	
	Uint32 Peer::getUploadRate() const 
	{
		return speed->uploadRate();
	}
	
	Uint32 Peer::getDownloadRate() const 
	{
		return speed->downloadRate();
	}
	
	bool Peer::readyToSend() const 
	{
		return true;
	}
	
	void Peer::updateSpeed()
	{
		speed->update();
	}
	
	bool Peer::isSnubbed() const
	{
		return snub_timer.getElapsedSinceUpdate() >= 60000;
	}

	bool Peer::isSeeder() const
	{
		return pieces.allOn();
	}

	QString Peer::getIPAddresss() const
	{
		if (sock)
			return sock->peerAddress().toString();
		else
			return QString::null;
	}
}

#include "peer.moc"
