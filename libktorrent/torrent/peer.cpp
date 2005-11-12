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

#include <kbufferedsocket.h>
#include <util/log.h>
#include <math.h>
#include "peer.h"
#include "chunk.h"
#include "speedestimater.h"
#include "upspeedestimater.h"
#include "piece.h"
#include "request.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "peerdownloader.h"
#include "peeruploader.h"
#include <util/functions.h>

namespace bt
{
	
	
	static Uint32 peer_id_counter = 1;
	
	Peer::Peer(KNetwork::KBufferedSocket* sock,const PeerID & peer_id,Uint32 num_chunks)
	: sock(sock),pieces(num_chunks),peer_id(peer_id)
	{
		id = peer_id_counter;
		peer_id_counter++;
		
		speed = new SpeedEstimater();
		up_speed = new UpSpeedEstimater();
		preader = new PacketReader(sock,speed);
		choked = am_choked = true;
		interested = am_interested = false;
		killed = false;
		downloader = new PeerDownloader(this);
		uploader = new PeerUploader(this);
		
		connect(sock,SIGNAL(closed()),this,SLOT(connectionClosed()));
		connect(sock,SIGNAL(readyRead()),this,SLOT(readyRead()));
		connect(sock,SIGNAL(gotError(int)),this,SLOT(error(int)));
		pwriter = new PacketWriter(this);
		time_choked = GetCurrentTime();

		connect_time = QTime::currentTime();
		connect(sock,SIGNAL(bytesWritten(int)),this,SLOT(dataWritten(int )));
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
		delete up_speed;
	}

	void Peer::connectionClosed() 
	{
		Out() << "Connection Closed" << endl;
		closeConnection();
		killed = true;
	} 
	
	void Peer::closeConnection()
	{
		sock->close();
	}
	
	void Peer::readyRead() 
	{
		if (killed) return;
		readPacket();
	}
	
	void Peer::error(int)
	{
		//Out() << "Error : " << err << endl;
		sock->close();
		killed = true;
	}

	void Peer::kill()
	{
		sock->close();
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
				bitSetRecieved(pieces);
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
				//	Out() << "REQUEST " << r.getIndex() << " " << r.getOffset() << endl;
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
		up_speed->writeBytes(len,record);
	}

	void Peer::dataWritten(int bytes)
	{
		up_speed->bytesWritten(bytes);
	}
	
	Uint32 Peer::getUploadRate() const 
	{
		return (Uint32)ceil(up_speed->uploadRate());
	}
	
	Uint32 Peer::getDownloadRate() const 
	{
		return (Uint32)ceil(speed->downloadRate());
	}
	
	bool Peer::readyToSend() const 
	{
		return true;
	}
	
	void Peer::updateSpeed()
	{
		speed->update();
		up_speed->update();
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

	Uint32 Peer::getTimeSinceLastPiece() const
	{
		return snub_timer.getElapsedSinceUpdate();
	}

	float Peer::percentAvailable() const
	{
		return (float)pieces.numOnBits() / (float)pieces.getNumBits() * 100.0;
	}

	void Peer::getStats(kt::PeerInterface::Stats & s)
	{
		s.choked = this->isChoked();
		s.client = peer_id.identifyClient();
		s.download_rate = this->getDownloadRate();
		s.upload_rate = this->getUploadRate();
		s.ip_addresss = this->getIPAddresss();
		s.perc_of_file = this->percentAvailable();
		s.snubbed = this->isSnubbed();
	}
}

#include "peer.moc"
