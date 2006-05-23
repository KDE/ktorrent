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
#include <math.h>
#include <util/log.h>
#include <util/functions.h>
#include <mse/streamsocket.h>

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


namespace bt
{

	
	
	static Uint32 peer_id_counter = 1;
	
	
	Peer::Peer(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 num_chunks,Uint32 support)
	: sock(sock),pieces(num_chunks),peer_id(peer_id)
	{
		id = peer_id_counter;
		peer_id_counter++;
		
		speed = new SpeedEstimater();
		up_speed = new UpSpeedEstimater();
		preader = new PacketReader(this,speed);
		choked = am_choked = true;
		interested = am_interested = false;
		killed = false;
		recieved_packet = false;
		downloader = new PeerDownloader(this);
		uploader = new PeerUploader(this);
		
		
		pwriter = new PacketWriter(this);
		time_choked = GetCurrentTime();
		time_unchoked = 0;
		
		connect_time = QTime::currentTime();
		sock->attachPeer(this);
		stats.client = peer_id.identifyClient();
		stats.ip_addresss = getIPAddresss();
		stats.choked = true;
		stats.download_rate = 0;
		stats.upload_rate = 0;
		stats.perc_of_file = 0;
		stats.snubbed = false;
		stats.dht_support = support & DHT_SUPPORT;
		stats.fast_extensions = support & FAST_EXT_SUPPORT;
		stats.bytes_downloaded = stats.bytes_uploaded = 0;
		stats.aca_score = 0.0;
		stats.evil = false;
		stats.has_upload_slot = false;
		stats.num_requests = 0;
		if (stats.ip_addresss == "0.0.0.0")
		{
			Out() << "No more 0.0.0.0" << endl;
			kill();
		}
		
		if (sock->bytesAvailable() > 0)
			recieved_packet = true;
		
		stats.encrypted = sock->encrypted();
	}


	Peer::~Peer()
	{
		if (sock)
			sock->detachPeer(this);
		
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
		// set a flag indicating that packets are ready
		recieved_packet = true;
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
		preader->update();
		
		if (!preader->ok())
			error(0);
		
		recieved_packet = false;
	}
	
	void Peer::packetReady(const Uint8* packet,Uint32 len)
	{
		if (killed) return;
		
		if (len == 0)
			return;
		const Uint8* tmp_buf = packet;
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
				
				if (choked)
					time_unchoked = GetCurrentTime();
				choked = false;
				break;
			case INTERESTED:
				if (len != 1)
				{
					Out() << "len err INTERESTED" << endl;
					error(0);
					return;
				}
				if (!interested)
				{
					interested = true;
					rerunChoker();
				}
				break;
			case NOT_INTERESTED:
				if (len != 1)
				{
					Out() << "len err NOT_INTERESTED" << endl;
					error(0);
					return;
				}
				if (interested)
				{
					interested = false;
					rerunChoker();
				}
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
				// less then 5 pieces and support for fast_extensions so enable allowed fast
				if (stats.fast_extensions && pieces.numOnBits() < 5)
					uploader->enableAllowedFast();
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
					stats.bytes_downloaded += (len - 9);
					// turn on evil bit
					if (stats.evil)
						stats.evil = false;
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
			case REJECT_REQUEST:
				if (len != 13)
				{
					Out() << "len err REJECT_REQUEST" << endl;
					error(0);
					return;
				}
				
				{
					Request r(ReadUint32(tmp_buf,1),
							  ReadUint32(tmp_buf,5),
							  ReadUint32(tmp_buf,9),
							  id);
					downloader->onRejected(r);
				}
				break;
			case PORT:
				if (len != 3)
				{
					Out() << "len err PORT" << endl;
					error(0);
					return;
				}
				
				{
					Uint16 port = ReadUint16(tmp_buf,1);
					Out() << "Got PORT packet : " << port << endl;
					gotPortPacket(getIPAddresss(),port);
				}
				break;
			case HAVE_ALL:
				if (len != 1)
				{
					Out() << "len err HAVE_ALL" << endl;
					error(0);
					return;
				}
				Out() << "HAVE_ALL" << endl;
				pieces.setAll(true);
				bitSetRecieved(pieces);
				break;
			case HAVE_NONE:
				if (len != 1)
				{
					Out() << "len err HAVE_NONE" << endl;
					error(0);
					return;
				}
				Out() << "HAVE_NONE" << endl;
				pieces.setAll(false);
				bitSetRecieved(pieces);
				// no pieces so give enable AF if peer supports it
				if (stats.fast_extensions)
					uploader->enableAllowedFast();
				break;
			case SUGGEST_PIECE:
				Out() << "SUGGEST_PIECE" << endl;
				// ignore suggestions for the moment
				break;
			case ALLOWED_FAST:
				if (len != 5)
				{
					Out() << "len err ALLOWED_FAST" << endl;
					error(0);
					return;
				}
				Out() << "ALLOWED_FAST " << ReadUint32(tmp_buf,1) << endl;
				downloader->addAllowedFastChunk(ReadUint32(tmp_buf,1));
				break;
		}
	}
	
	void Peer::sendData(const Uint8* data,Uint32 len,bool proto)
	{
		if (killed) return;
		
		sock->sendData(data,len);
		up_speed->writeBytes(len,proto);
	}
	
	Uint32 Peer::readData(Uint8* buf,Uint32 len)
	{
		if (killed) return 0;
		
		return sock->readData(buf,len);
	}
	
	Uint32 Peer::bytesAvailable() const
	{
		return sock->bytesAvailable();
	}

	void Peer::dataWritten(int bytes)
	{
	//	Out() << "dataWritten " << bytes << endl;
		up_speed->bytesWritten(bytes);
	}
	
	Uint32 Peer::getUploadRate() const 
	{
		return (Uint32)ceil(up_speed->uploadRate());
	}
	
	Uint32 Peer::getProtocolUploadRate() const
	{
		return (Uint32)ceil(up_speed->protocollOverhead());
	}
	
	Uint32 Peer::getDownloadRate() const 
	{
		return (Uint32)ceil(speed->downloadRate());
	}
	
	bool Peer::readyToSend() const 
	{
		return true;
	}
	
	void Peer::update()
	{
		if (recieved_packet)
			readPacket();
		speed->update();
		up_speed->update();
	}
	
	bool Peer::isSnubbed() const
	{
		// 4 minutes
		return snub_timer.getElapsedSinceUpdate() >= 2*60*1000;
	}

	bool Peer::isSeeder() const
	{
		return pieces.allOn();
	}

	QString Peer::getIPAddresss() const
	{
		if (sock)
			return sock->getIPAddress();
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

	const kt::PeerInterface::Stats & Peer::getStats() const
	{
		stats.choked = this->isChoked();
		stats.download_rate = this->getDownloadRate();
		stats.upload_rate = this->getUploadRate();
		stats.perc_of_file = this->percentAvailable();
		stats.snubbed = this->isSnubbed();
		stats.num_requests = uploader->getNumRequests();
		return stats;
	}
	
	void Peer::setACAScore(double s)
	{
		stats.aca_score = s;
	}
	
	void Peer::choke()
	{
		if (am_choked)
			return;
		
		pwriter->sendChoke();
		if (stats.fast_extensions)
		{
			// send rejects for all queued up packets
			uploader->rejectAll();
		}
	}
}

#include "peer.moc"
