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

#include "metadatadownload.h"
#include <util/log.h>
#include <bcodec/bencoder.h>
#include <peer/utmetadata.h>

namespace bt
{
	
	MetadataDownload::MetadataDownload(UTMetaData* ext, Uint32 size) : ext(ext),total_size(size)
	{
		metadata.resize(size);
		Uint32 num_pieces = size / METADATA_PIECE_SIZE;
		if (size % METADATA_PIECE_SIZE > 0)
			num_pieces++;
		
		pieces = BitSet(num_pieces);
		download(0);
	}

	MetadataDownload::~MetadataDownload()
	{
	}

	void MetadataDownload::reject(Uint32 piece)
	{
		Out(SYS_GEN|LOG_NOTICE) << "Metadata download, piece " << piece << " rejected" << endl;
		downloadNext();
	}

	bool MetadataDownload::data(Uint32 piece, const QByteArray& piece_data)
	{
		// validate data
		if (piece >= pieces.getNumBits())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Metadata download, received piece " << piece << " is invalid " << endl;
			downloadNext();
			return false;
		}
		
		int size = METADATA_PIECE_SIZE;
		if (piece == pieces.getNumBits() - 1 && total_size % METADATA_PIECE_SIZE > 0)
			size = total_size % METADATA_PIECE_SIZE ;
		
		if (size != piece_data.size())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Metadata download, received piece " << piece << " has the wrong size " << endl;
			downloadNext();
			return false;
		}
		
		// piece fits, so copy into data
		Out(SYS_GEN|LOG_NOTICE) << "Metadata download, dowloaded " << piece << endl;
		Uint32 off = piece * METADATA_PIECE_SIZE;
		memcpy(metadata.data() + off,piece_data.data(),size);
		pieces.set(piece,true);
		
		if (!pieces.allOn())
			downloadNext();
		
		return pieces.allOn();
	}

	void MetadataDownload::download(Uint32 piece)
	{
		Out(SYS_GEN|LOG_NOTICE) << "Metadata download, requesting " << piece << endl;
		QByteArray request;
		BEncoder enc(new BEncoderBufferOutput(request));
		enc.beginDict();
		enc.write(QString("msg_type")); enc.write((bt::Uint32)0);
		enc.write(QString("piece")); enc.write((bt::Uint32)piece);
		enc.end();
		ext->sendPacket(request);
	}

	void MetadataDownload::downloadNext()
	{
		for (Uint32 i = 0;i < pieces.getNumBits();i++)
			if (!pieces.get(i))
				download(i);
	}

}

