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

#ifndef BT_UTMETADATA_H
#define BT_UTMETADATA_H

#include <peer/peerprotocolextension.h>

namespace bt
{
	class MetadataDownload;
	class BDictNode;
	class Peer;
	class Torrent;
	
	/**
		Handles ut_metadata extension
	*/
	class BTCORE_EXPORT UTMetaData : public PeerProtocolExtension
	{
	public:
		UTMetaData(const Torrent & tor,bt::Uint32 id,Peer* peer);
		virtual ~UTMetaData();
		
		/**
			Handle a metadata packet
		*/
		void handlePacket(const bt::Uint8* packet, Uint32 size);
		
		/**
			Set the reported metadata size
		*/
		void setReportedMetadataSize(Uint32 metadata_size);
		
	private:
		void request(BDictNode* dict);
		void reject(BDictNode* dict);
		void data(BDictNode* dict,const QByteArray & piece_data);
		void sendReject(int piece);
		void sendData(int piece,int total_size,const QByteArray & data);
		void startDownload();

	private:
		const Torrent & tor;
		Uint32 reported_metadata_size;
		MetadataDownload* download;
	};

}

#endif // BT_UTMETADATA_H
