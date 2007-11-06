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
#include <math.h>
#include <util/log.h>
#include <util/functions.h>
#include <net/address.h>
#include <mse/streamsocket.h>

#include "peer.h"
#include "chunk.h"
#include "piece.h"
#include "request.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "peerdownloader.h"
#include "peeruploader.h"
#include "bdecoder.h"
#include "bnode.h"
#include "utpex.h"
#include "server.h"

using namespace net;

namespace bt
{

	
	
	static Uint32 peer_id_counter = 1;
	
	
	Peer::Peer(mse::StreamSocket* sock,const PeerID & peer_id,
			   Uint32 num_chunks,Uint32 chunk_size,Uint32 support,bool local)
	: sock(sock),pieces(num_chunks),peer_id(peer_id)
	{
		id = peer_id_counter;
		peer_id_counter++;
		
		ut_pex = 0;
		preader = new PacketReader(this);
		choked = am_choked = true;
		interested = am_interested = false;
		killed = false;
		downloader = new PeerDownloader(this,chunk_size);
		uploader = new PeerUploader(this);
		
		
		pwriter = new PacketWriter(this);
		time_choked = GetCurrentTime();
		time_unchoked = 0;
		
		connect_time = QTime::currentTime();
		//sock->attachPeer(this);
		stats.client = peer_id.identifyClient();
		stats.ip_address = getIPAddresss();
		stats.choked = true;
		stats.download_rate = 0;
		stats.upload_rate = 0;
		stats.perc_of_file = 0;
		stats.snubbed = false;
		stats.dht_support = support & DHT_SUPPORT;
		stats.fast_extensions = support & FAST_EXT_SUPPORT;
		stats.extension_protocol = support & EXT_PROT_SUPPORT;
		stats.bytes_downloaded = stats.bytes_uploaded = 0;
		stats.aca_score = 0.0;
		stats.evil = false;
		stats.has_upload_slot = false;
		stats.num_up_requests = stats.num_down_requests = 0;
		stats.encrypted = sock->encrypted();
		stats.local = local;
		if (stats.ip_address == "0.0.0.0")
		{
			Out(SYS_CON|LOG_DEBUG) << "No more 0.0.0.0" << endl;
			kill();
		}
		else
		{
			sock->startMonitoring(preader,pwriter);
		}
		pex_allowed = stats.extension_protocol;
		utorrent_pex_id = 0;
	}


	Peer::~Peer()
	{
		delete ut_pex;
		delete uploader;
		delete downloader;
		delete sock;
		delete pwriter;
		delete preader;
	}
	
	void Peer::closeConnection()
	{
		sock->close();
	}
	

	void Peer::kill()
	{
		sock->close();
		killed = true;
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
					kill();
					return;
				}
				
				if (!choked)
				{
					time_choked = GetCurrentTime();
				}
				choked = true;
				downloader->choked();
				break;
			case UNCHOKE:
				if (len != 1)
				{
					Out() << "len err UNCHOKE" << endl;
					kill();
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
					kill();
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
					kill();
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
					kill();
				}
				else
				{
					Uint32 ch = ReadUint32(tmp_buf,1);
					if (ch < pieces.getNumBits())
					{
						haveChunk(this,ch);
						pieces.set(ch,true);
					}
					else
					{
						Out(SYS_CON|LOG_NOTICE) << "Received invalid have value, kicking peer" << endl;
						kill();
					}
				}
				break;
			case BITFIELD:
				if (len != 1 + pieces.getNumBytes())
				{
					Out() << "len err BITFIELD" << endl;
					kill();
					return;
				}
				
				pieces = BitSet(tmp_buf+1,pieces.getNumBits());
				bitSetRecieved(pieces);
				break;
			case REQUEST:
				if (len != 13)
				{
					Out() << "len err REQUEST" << endl;
					kill();
					return;
				}
				
				{
					Request r(
							ReadUint32(tmp_buf,1),
							ReadUint32(tmp_buf,5),
							ReadUint32(tmp_buf,9),
							id);
					
					if (!am_choked)
						uploader->addRequest(r);
					else if (stats.fast_extensions)
						pwriter->sendReject(r);
				//	Out() << "REQUEST " << r.getIndex() << " " << r.getOffset() << endl;
				}
				break;
			case PIECE:
				if (len < 9)
				{
					Out() << "len err PIECE" << endl;
					kill();
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
					kill();
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
					kill();
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
					kill();
					return;
				}
				
				{
					Uint16 port = ReadUint16(tmp_buf,1);
				//	Out() << "Got PORT packet : " << port << endl;
					gotPortPacket(getIPAddresss(),port);
				}
				break;
			case HAVE_ALL:
				if (len != 1)
				{
					Out() << "len err HAVE_ALL" << endl;
					kill();
					return;
				}
				pieces.setAll(true);
				bitSetRecieved(pieces);
				break;
			case HAVE_NONE:
				if (len != 1)
				{
					Out() << "len err HAVE_NONE" << endl;
					kill();
					return;
				}
				pieces.setAll(false);
				bitSetRecieved(pieces);
				break;
			case SUGGEST_PIECE:
				// ignore suggestions for the moment
				break;
			case ALLOWED_FAST:
				// we no longer support this, so do nothing
				break;
			case EXTENDED:
				handleExtendedPacket(packet,len);
				break;
		}
	}
	
	void Peer::handleExtendedPacket(const Uint8* packet,Uint32 size)
	{
		if (size <= 2 || packet[1] > 1)
			return;
		
		if (packet[1] == 1)
		{
			if (ut_pex)
				ut_pex->handlePexPacket(packet,size);
			return;
		}
		
		QByteArray tmp;
		tmp.setRawData((const char*)packet,size);
		BNode* node = 0;
		try
		{
			BDecoder dec(tmp,false,2);
			node = dec.decode();
			if (node && node->getType() == BNode::DICT)
			{
				BDictNode* dict = (BDictNode*)node;
				
				// handshake packet, so just check if the peer supports ut_pex
				dict = dict->getDict("m");
				BValueNode* val = 0;
				if (dict && (val = dict->getValue("ut_pex")))
				{
					utorrent_pex_id = val->data().toInt();
					if (ut_pex)
					{
						if (utorrent_pex_id > 0)
							ut_pex->changeID(utorrent_pex_id);
						else
						{
							// id 0 means disabled
							delete ut_pex;
							ut_pex = 0;
						}
					}
					else if (!ut_pex && utorrent_pex_id != 0 && pex_allowed) 
					{
						// Don't create  it when the id is 0
						ut_pex = new UTPex(this,utorrent_pex_id);
					}	
				}
			}
		}
		catch (...)
		{
			// just ignore invalid packets
			Out(SYS_CON|LOG_DEBUG) << "Invalid extended packet" << endl;
		}
		delete node;
		tmp.resetRawData((const char*)packet,size);
	}
	
	Uint32 Peer::sendData(const Uint8* data,Uint32 len)
	{
		if (killed) return 0;
		
		Uint32 ret = sock->sendData(data,len);
		if (!sock->ok())
			kill();
		
		return ret;
	}
	
	Uint32 Peer::readData(Uint8* buf,Uint32 len)
	{
		if (killed) return 0;
		
		Uint32 ret = sock->readData(buf,len);
		
		if (!sock->ok())
			kill();
		
		return ret;
	}
	
	Uint32 Peer::bytesAvailable() const
	{
		return sock->bytesAvailable();
	}

	void Peer::dataWritten(int )
	{
	//	Out() << "dataWritten " << bytes << endl;
		
	}
	
	Uint32 Peer::getUploadRate() const 
	{
		if (sock)
			return (Uint32)ceil(sock->getUploadRate());
		else
			return 0;
	}
	
	Uint32 Peer::getDownloadRate() const 
	{
		if (sock)
			return (Uint32)ceil(sock->getDownloadRate());
		else
			return 0;
	}
	
	bool Peer::readyToSend() const 
	{
		return true;
	}
	
	void Peer::update(PeerManager* pman)
	{
		if (killed)
			return;
		
		if (!sock->ok() || !preader->ok())
		{
			Out(SYS_CON|LOG_DEBUG) << "Connection closed" << endl;
			kill();
			return;
		}
		
		preader->update();
		
		Uint32 data_bytes = pwriter->getUploadedDataBytes();
		
		if (data_bytes > 0)
		{
			stats.bytes_uploaded += data_bytes;
			uploader->addUploadedBytes(data_bytes);
		}
		
		if (ut_pex && ut_pex->needsUpdate())
			ut_pex->update(pman);
	}
	
	bool Peer::isSnubbed() const
	{
		// 4 minutes
		return snub_timer.getElapsedSinceUpdate() >= 2*60*1000 && stats.num_down_requests > 0;
	}

	bool Peer::isSeeder() const
	{
		return pieces.allOn();
	}

	QString Peer::getIPAddresss() const
	{
		if (sock)
			return sock->getRemoteIPAddress();
		else
			return QString::null;
	}
	
	Uint16 Peer::getPort() const
	{
		if (!sock)
			return 0;
		else
			return sock->getRemotePort();
	}
	
	net::Address Peer::getAddress() const
	{
		if (!sock)
			return net::Address();
		else
			return sock->getRemoteAddress();
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
		stats.num_up_requests = uploader->getNumRequests();
		stats.num_down_requests = downloader->getNumRequests();
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
		uploader->clearAllRequests();
	}
	
	void Peer::emitPortPacket()
	{
		gotPortPacket(sock->getRemoteIPAddress(),sock->getRemotePort());
	}
	
	void Peer::emitPex(const QByteArray & data)
	{
		pex(data);
	}
	
	void Peer::setPexEnabled(bool on)
	{
		if (!stats.extension_protocol)
			return;
		
		// send extension protocol handshake
		bt::Uint16 port = Globals::instance().getServer().getPortInUse();
		
		if (ut_pex && !on)
		{
			delete ut_pex;
			ut_pex = 0;
		}
		else if (!ut_pex && on && utorrent_pex_id > 0)
		{
			// if the other side has enabled it to, create a new UTPex object
			ut_pex = new UTPex(this,utorrent_pex_id);
		}
		
		pwriter->sendExtProtHandshake(port,on);
		
		pex_allowed = on;
	}
	
	void Peer::setGroupIDs(Uint32 up_gid,Uint32 down_gid)
	{
		sock->setGroupIDs(up_gid,down_gid);
	}
}

#include "peer.moc"
