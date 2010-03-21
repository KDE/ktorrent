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
#include "peer.h"

#include <math.h>
#include <btversion.h>
#include <util/log.h>
#include <util/functions.h>
#include <net/address.h>
#include <mse/streamsocket.h>
#include <diskio/chunk.h>
#include <download/piece.h>
#include <download/request.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <torrent/server.h>
#include <torrent/torrent.h>
#include "packetreader.h"
#include "packetwriter.h"
#include "peerdownloader.h"
#include "peeruploader.h"
#include "utpex.h"
#include "peermanager.h"
#include <net/reverseresolver.h>
#include "utmetadata.h"


using namespace net;

namespace bt
{

	static Uint32 peer_id_counter = 1;
	bool Peer::resolve_hostname = true;
	
	
	Peer::Peer(mse::StreamSocket* sock,const PeerID & peer_id,
			   Uint32 num_chunks,Uint32 chunk_size,Uint32 support,bool local,PeerManager* pman)
	: sock(sock),pieces(num_chunks),peer_id(peer_id),pman(pman)
	{
		id = peer_id_counter;
		peer_id_counter++;
		ut_pex_id = 0;
		preader = new PacketReader(this);
		stats.choked = true;
		stats.interested = stats.am_interested = false;
		killed = false;
		downloader = new PeerDownloader(this,chunk_size);
		uploader = new PeerUploader(this);
		
		
		stalled_timer.update();
		pwriter = new PacketWriter(this);
		time_choked = CurrentTime();
		time_unchoked = 0;
		
		connect_time = QTime::currentTime();
		//sock->attachPeer(this);
		stats.client = peer_id.identifyClient();
		stats.ip_address = getIPAddresss();
		stats.choked = true;
		paused = false;
		stats.interested = false;
		stats.am_interested = false;
		stats.download_rate = 0;
		stats.upload_rate = 0;
		stats.perc_of_file = 0;
		stats.snubbed = false;
		stats.dht_support = support & DHT_SUPPORT;
		stats.fast_extensions = support & FAST_EXT_SUPPORT;
		stats.extension_protocol = support & EXT_PROT_SUPPORT;
		stats.bytes_downloaded = stats.bytes_uploaded = 0;
		stats.aca_score = 0.0;
		stats.has_upload_slot = false;
		stats.num_up_requests = stats.num_down_requests = 0;
		stats.encrypted = sock->encrypted();
		stats.local = local;
		stats.max_request_queue = 0;
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
		extensions.setAutoDelete(true);
		
		if (resolve_hostname)
		{
			net::ReverseResolver* res = new net::ReverseResolver();
			connect(res,SIGNAL(resolved(QString)),this,SLOT(resolved(QString)),Qt::QueuedConnection);
			res->resolveAsync(sock->getRemoteAddress());
		}
	}


	Peer::~Peer()
	{
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
		Uint8 type = tmp_buf[0];
		switch (type)
		{
			case CHOKE:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err CHOKE" << endl;
					kill();
					return;
				}
				
				if (!stats.choked)
				{
					time_choked = CurrentTime();
				}
				stats.choked = true;
				downloader->choked();
				break;
			case UNCHOKE:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err UNCHOKE" << endl;
					kill();
					return;
				}
				
				if (stats.choked)
					time_unchoked = CurrentTime();
				stats.choked = false;
				break;
			case INTERESTED:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err INTERESTED" << endl;
					kill();
					return;
				}
				if (!stats.interested)
				{
					stats.interested = true;
					pman->rerunChoker();
				}
				break;
			case NOT_INTERESTED:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err NOT_INTERESTED" << endl;
					kill();
					return;
				}
				if (stats.interested)
				{
					stats.interested = false;
					pman->rerunChoker();
				}
				break;
			case HAVE:
				if (len != 5)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err HAVE" << endl;
					kill();
				}
				else
				{
					Uint32 ch = ReadUint32(tmp_buf,1);
					if (ch < pieces.getNumBits())
					{
						pman->have(this,ch);
						pieces.set(ch,true);
					}
					else if (pman->getTorrent().isLoaded())
					{
						Out(SYS_CON|LOG_NOTICE) << "Received invalid have value, kicking peer" << endl;
						kill();
					}
				}
				break;
			case BITFIELD:
				if (len != 1 + pieces.getNumBytes())
				{
					if (pman->getTorrent().isLoaded())
					{
						Out(SYS_CON|LOG_DEBUG) << "len err BITFIELD" << endl;
						kill();
					}
					return;
				}
				
				pieces = BitSet(tmp_buf+1,pieces.getNumBits());
				pman->bitSetReceived(this,pieces);
				break;
			case REQUEST:
				if (len != 13)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err REQUEST" << endl;
					kill();
					return;
				}
				
				{
					Request r(
							ReadUint32(tmp_buf,1),
							ReadUint32(tmp_buf,5),
							ReadUint32(tmp_buf,9),
							downloader);
					
					if (stats.has_upload_slot)
						uploader->addRequest(r);
					else if (stats.fast_extensions)
						pwriter->sendReject(r);
				//	Out(SYS_CON|LOG_DEBUG) << "REQUEST " << r.getIndex() << " " << r.getOffset() << endl;
				}
				break;
			case PIECE:
				if(paused)
					return;
				if (len < 9)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err PIECE" << endl;
					kill();
					return;
				}
				
				snub_timer.update();
				
				{
					stats.bytes_downloaded += (len - 9);
					Piece p(ReadUint32(tmp_buf,1),
							ReadUint32(tmp_buf,5),
							len - 9,downloader,tmp_buf+9);
					downloader->piece(p);
					pman->pieceReceived(p);
					downloader->update();
				}
				break;
			case CANCEL:
				if (len != 13)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err CANCEL" << endl;
					kill();
					return;
				}
				
				{
					Request r(ReadUint32(tmp_buf,1),
							  ReadUint32(tmp_buf,5),
							  ReadUint32(tmp_buf,9),
							  downloader);
					uploader->removeRequest(r);
				}
				break;
			case REJECT_REQUEST:
				if (len != 13)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err REJECT_REQUEST" << endl;
					kill();
					return;
				}
				
				{
					Request r(ReadUint32(tmp_buf,1),
							  ReadUint32(tmp_buf,5),
							  ReadUint32(tmp_buf,9),
							  downloader);
					downloader->onRejected(r);
				}
				break;
			case PORT:
				if (len != 3)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err PORT" << endl;
					kill();
					return;
				}
				
				{
					Uint16 port = ReadUint16(tmp_buf,1);
				//	Out(SYS_CON|LOG_DEBUG) << "Got PORT packet : " << port << endl;
					pman->portPacketReceived(getIPAddresss(),port);
				}
				break;
			case HAVE_ALL:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err HAVE_ALL" << endl;
					kill();
					return;
				}
				pieces.setAll(true);
				pman->bitSetReceived(this,pieces);
				break;
			case HAVE_NONE:
				if (len != 1)
				{
					Out(SYS_CON|LOG_DEBUG) << "len err HAVE_NONE" << endl;
					kill();
					return;
				}
				pieces.setAll(false);
				pman->bitSetReceived(this,pieces);
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
	
	void Peer::pause()
	{
		if (paused)
			return;
		
		downloader->cancelAll();
		// choke the peer and tell it we are not interested
		choke();
		pwriter->sendNotInterested();
		paused = true;
	}
	
	void Peer::unpause()
	{
		paused = false;
	}

	void Peer::handleExtendedPacket(const Uint8* packet,Uint32 size)
	{
		if (size <= 2)
			return;
		
		PeerProtocolExtension* ext = extensions.find(packet[1]);
		if (ext)
		{
			ext->handlePacket(packet,size);
		}
		else if (packet[1] == 0)
		{
			handleExtendedHandshake(packet,size);
		}
	}
	
	void Peer::handleExtendedHandshake(const Uint8* packet,Uint32 size)
	{
		QByteArray tmp = QByteArray::fromRawData((const char*)packet,size);
		BNode* node = 0;
		try
		{
			BDecoder dec(tmp,false,2);
			node = dec.decode();
			if (!node || node->getType() != BNode::DICT)
			{
				delete node;
				return;
			}
			
			BDictNode* dict = (BDictNode*)node;
			BDictNode* mdict = dict->getDict(QString("m"));
			if (!mdict)
			{
				delete node;
				return;
			}
			
			BValueNode* val = 0;
			
			if ((val = mdict->getValue("ut_pex")) && UTPex::isEnabled())
			{
				// ut_pex packet
				ut_pex_id = val->data().toInt();
				if (ut_pex_id == 0)
				{
					extensions.erase(UT_PEX_ID);
				}
				else
				{
					PeerProtocolExtension* ext = extensions.find(UT_PEX_ID);
					if (ext)
						ext->changeID(ut_pex_id);
					else if (pex_allowed)
						extensions.insert(UT_PEX_ID,new UTPex(this,ut_pex_id));
				}
			}
				
			if ((val = mdict->getValue("ut_metadata")))
			{
				// meta data
				Uint32 ut_metadata_id = val->data().toInt();
				if (ut_metadata_id == 0) // disabled by other side
				{
					extensions.erase(UT_METADATA_ID);
				}
				else
				{
					PeerProtocolExtension* ext = extensions.find(UT_METADATA_ID);
					if (ext)
					{
						ext->changeID(ut_metadata_id);
					}
					else
					{
						int metadata_size = 0;
						if (dict->getValue("metadata_size"))
							metadata_size = dict->getInt("metadata_size");
						
						UTMetaData* md = new UTMetaData(pman->getTorrent(),ut_metadata_id,this);
						md->setReportedMetadataSize(metadata_size);
						extensions.insert(UT_METADATA_ID,md);
					}
				}
			}
			
			if ((val = dict->getValue("reqq")))
			{
				stats.max_request_queue = val->data().toInt();
			}
		}
		catch (...)
		{
			// just ignore invalid packets
			Out(SYS_CON|LOG_DEBUG) << "Invalid extended packet" << endl;
		}
		delete node;
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
	
	void Peer::update()
	{
		if (killed)
			return;
		
		if (!sock->ok() || !preader->ok())
		{
			Out(SYS_CON|LOG_DEBUG) << "Connection closed" << endl;
			kill();
			return;
		}
		
		sock->updateSpeeds();
		preader->update();
		
		Uint32 data_bytes = pwriter->getUploadedDataBytes();
		
		if (data_bytes > 0)
		{
			stats.bytes_uploaded += data_bytes;
			uploader->addUploadedBytes(data_bytes);
		}
		
		if (!paused)
		{
			PtrMap<Uint32,PeerProtocolExtension>::iterator i = extensions.begin();
			while (i != extensions.end())
			{
				if (i->second->needsUpdate())
					i->second->update();
				i++;
			}
		}
		
		// if no data is being sent or recieved, and there are pending requests
		// increment the connection stalled timer
		if (getUploadRate() > 100 || getDownloadRate() > 100 || 
		   (uploader->getNumRequests() == 0 && downloader->getNumRequests() == 0) )
			stalled_timer.update();
	}
	
	bool Peer::isStalled() const
	{
		return stalled_timer.getElapsedSinceUpdate() >= 2*60*1000;
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
			return QString();
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
		// calculation needs to use bytes, instead of chunks, because
		// the last chunk can have a different size
		const Torrent & tor = pman->getTorrent();
		Uint64 bytes = 0; 
		if (pieces.get(tor.getNumChunks() - 1))
			bytes = tor.getChunkSize() * (pieces.numOnBits() - 1) + tor.getLastChunkSize();
		else
			bytes = tor.getChunkSize() * pieces.numOnBits();
		
		Uint64 tbytes = tor.getChunkSize() * (pieces.getNumBits() - 1) + tor.getLastChunkSize();
		return (float)bytes / (float)tbytes * 100.0;
	}

	const PeerInterface::Stats & Peer::getStats() const
	{
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
		if (!stats.has_upload_slot)
			return;
		
		pwriter->sendChoke();
		uploader->clearAllRequests();
	}
	
	void Peer::emitPortPacket()
	{
		pman->portPacketReceived(sock->getRemoteIPAddress(),sock->getRemotePort());
	}
	
	void Peer::emitPex(const QByteArray & data)
	{
		pman->pex(data);
	}
	
	void Peer::setPexEnabled(bool on)
	{
		if (!stats.extension_protocol)
			return;
		
		PeerProtocolExtension* ext = extensions.find(UT_PEX_ID);
		if (ext && (!on || !UTPex::isEnabled()))
		{
			extensions.erase(UT_PEX_ID);
		}
		else if (!ext && on && ut_pex_id > 0 && UTPex::isEnabled())
		{
			// if the other side has enabled it to, create a new UTPex object
			extensions.insert(UT_PEX_ID,new UTPex(this,ut_pex_id));
		}
		
		pex_allowed = on;
	}
	
	void Peer::sendExtProtHandshake(Uint16 port, Uint32 metadata_size)
	{
		if (!stats.extension_protocol)
			return;
		
		QByteArray arr;
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		enc.write(QString("m")); 
		// supported messages
		enc.beginDict();
		enc.write(QString("ut_pex"));enc.write((Uint32)(pex_allowed ? UT_PEX_ID : 0));
		enc.write(QString("ut_metadata")); enc.write(UT_METADATA_ID);
		enc.end();
		if (port > 0)
		{
			enc.write(QString("p")); 
			enc.write((Uint32)port);
		}
		enc.write("reqq");
		enc.write((bt::Uint32)250);
		if (metadata_size)
		{
			enc.write(QString("metadata_size")); 
			enc.write(metadata_size);
		}
		
		enc.write(QString("v")); enc.write(bt::GetVersionString());
		enc.end();
		pwriter->sendExtProtMsg(0,arr);
	}

	
	void Peer::setGroupIDs(Uint32 up_gid,Uint32 down_gid)
	{
		sock->setGroupIDs(up_gid,down_gid);
	}
		
	void Peer::resolved(const QString & hinfo)
	{
		stats.hostname = hinfo;
	}
	
	void Peer::setResolveHostnames(bool on)
	{
		resolve_hostname = on;
	}
	
	void Peer::emitMetadataDownloaded(const QByteArray& data)
	{
		emit metadataDownloaded(data);
	}

	bool Peer::hasWantedChunks(const bt::BitSet& wanted_chunks) const
	{
		BitSet bs = pieces;
		bs.andBitSet(wanted_chunks);
		return bs.numOnBits() > 0;
	}

}

#include "peer.moc"
