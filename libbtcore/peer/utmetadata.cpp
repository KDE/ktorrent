/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#include "utmetadata.h"
#include <QByteArray>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include <bcodec/bencoder.h>
#include "peer.h"
#include "packetwriter.h"

namespace bt
{
	
	UTMetaData::UTMetaData(const Torrent & tor,bt::Uint32 id,Peer* peer) 
		: PeerProtocolExtension(id,peer),tor(tor)
	{

	}

	UTMetaData::~UTMetaData()
	{

	}

	void UTMetaData::handlePacket(const bt::Uint8* packet, Uint32 size)
	{
		QByteArray tmp = QByteArray::fromRawData((const char*)packet,size);
		BNode* node = 0;
		try
		{
			BDecoder dec(tmp,false,2);
			node = dec.decode();
			if (!node || !node->getType() == BNode::DICT)
			{
				delete node;
				return;
			}
			
			BDictNode* dict = (BDictNode*)node;
			int type = dict->getInt("msg_type");
			switch (type)
			{
				case 0: // request
					request(dict);
					break;
				case 1: // data
					data(dict);
					break;
				case 2: // reject
					reject(dict);
					break;
			}
		}
		catch (...)
		{
			Out(SYS_CON|LOG_DEBUG) << "Invalid metadata packet" << endl;
		}
		delete node;
	}
	
	const int METADATA_PIECE_SIZE = 16 * 1024;

	void UTMetaData::data(BDictNode* dict)
	{

	}

	void UTMetaData::reject(BDictNode* dict)
	{

	}

	void UTMetaData::request(BDictNode* dict)
	{
		int piece = dict->getInt("piece");
		const QByteArray & md = tor.getMetaData();
		int num_pieces = md.size() / METADATA_PIECE_SIZE + (md.size() % METADATA_PIECE_SIZE == 0 ? 0 : 1);
		
		Out(SYS_CON|LOG_DEBUG) << "Received request for metadata piece " << piece << endl;
		if (piece < 0 || piece >= num_pieces)
		{
			sendReject(piece);
			return;
		}
		
		int off = piece * METADATA_PIECE_SIZE;
		int last_len = (md.size() % METADATA_PIECE_SIZE == 0) ? METADATA_PIECE_SIZE : (md.size() % METADATA_PIECE_SIZE);
		int len = piece == num_pieces - 1 ? last_len : METADATA_PIECE_SIZE;
		sendData(piece,md.size(),md.mid(off,len));
	}
	
	void UTMetaData::sendReject(int piece)
	{
		QByteArray data;
		BEncoder enc(new BEncoderBufferOutput(data));
		enc.beginDict();
		enc.write(QString("msg_type")); enc.write((bt::Uint32)2);
		enc.write(QString("piece")); enc.write((bt::Uint32)piece);
		enc.end();
		sendPacket(data);
	}
	
	void UTMetaData::sendData(int piece, int total_size, const QByteArray& data)
	{
		Out(SYS_CON|LOG_DEBUG) << "Sending metadata piece " << piece << endl;
		QByteArray tmp;
		BEncoder enc(new BEncoderBufferOutput(tmp));
		enc.beginDict();
		enc.write(QString("msg_type")); enc.write((bt::Uint32)1);
		enc.write(QString("piece")); enc.write((bt::Uint32)piece);
		enc.write(QString("total_size")); enc.write((bt::Uint32)total_size);
		enc.end();
		tmp.append(data);
		sendPacket(tmp);
	}


}

