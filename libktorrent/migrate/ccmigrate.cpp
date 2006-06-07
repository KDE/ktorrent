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
#include <klocale.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/bitset.h>
#include <util/fileops.h>
#include <torrent/downloader.h>
#include <torrent/torrent.h>
#include <torrent/globals.h>
#include <torrent/chunkdownload.h>
#include <ktversion.h>
#include "ccmigrate.h"

namespace bt
{
	bool IsPreMMap(const QString & current_chunks)
	{
		File fptr;
		if (!fptr.open(current_chunks,"rb"))
			return false;

		CurrentChunksHeader chdr;
		fptr.read(&chdr,sizeof(CurrentChunksHeader));
		if (chdr.magic != CURRENT_CHUNK_MAGIC)
		{
			// magic number not good, so pre
			return true;
		}
		
		if (chdr.major >= 2 || (chdr.major == 1 && chdr.minor >= 2))
		{
			// version number is 1.2 or greater
			return false;
		}
		
		return false;
	}
	
	static bool MigrateChunk(const Torrent & tor,File & new_cc,File & old_cc)
	{
		Uint32 ch = 0;
		old_cc.read(&ch,sizeof(Uint32));
		
		Out() << "Migrating chunk " << ch << endl;
		if (ch >= tor.getNumChunks())
			return false;
			
			// calculate the size
		Uint32 csize = 0;
		if (ch == tor.getNumChunks() - 1)
		{
				// ch is the last chunk, so it might have a different size
			csize = tor.getFileLength() % tor.getChunkSize();
			if (ch == 0)
				csize = tor.getChunkSize();
		}
		else
		{
			csize = tor.getChunkSize();
		}
			
		// calculate the number of pieces
		Uint32 num_pieces = csize / MAX_PIECE_LEN;
		if (csize % MAX_PIECE_LEN > 0)
			num_pieces++;
			
		// load the pieces array
		Array<bool> pieces(num_pieces);
		old_cc.read(pieces,sizeof(bool)*num_pieces);
		
		// convert bool array to bitset
		BitSet pieces_bs(num_pieces);
		for (Uint32 i = 0;i < num_pieces;i++)
			pieces_bs.set(i,pieces[i]);
			
		// load the actual data
		Array<Uint8> data(csize);
		old_cc.read(data,csize);
			
		// write to the new file
		ChunkDownloadHeader hdr;
		hdr.index = ch;
		hdr.num_bits = num_pieces;
		hdr.buffered = 1; // by default we will use buffered chunks
		// save the chunk header
		new_cc.write(&hdr,sizeof(ChunkDownloadHeader));
		// save the bitset
		new_cc.write(pieces_bs.getData(),pieces_bs.getNumBytes());
		new_cc.write(data,csize);
		return true;
	}

	static void MigrateCC(const Torrent & tor,const QString & current_chunks)
	{
		Out() << "Migrating current_chunks file " << current_chunks << endl;
		// open the old current_chunks file
		File old_cc;
		if (!old_cc.open(current_chunks,"rb"))
			throw Error(i18n("Cannot open file %1 : %2").arg(current_chunks).arg(old_cc.errorString()));
		
		// open a new file in the /tmp dir
		File new_cc;
		QString tmp = current_chunks + ".tmp";
		if (!new_cc.open(tmp,"wb"))
			throw Error(i18n("Cannot open file %1 : %2").arg(tmp).arg(old_cc.errorString()));
		
		// read the number of chunks
		Uint32 num = 0;
		old_cc.read(&num,sizeof(Uint32));
		Out() << "Found " << num << " chunks" << endl;
		
		// write the new current_chunks header
		CurrentChunksHeader hdr;
		hdr.magic = CURRENT_CHUNK_MAGIC;
		hdr.major = kt::MAJOR;
		hdr.minor = kt::MINOR;
		hdr.num_chunks = num;
		new_cc.write(&hdr,sizeof(CurrentChunksHeader));

		for (Uint32 i = 0;i < num;i++)
		{
			if (!MigrateChunk(tor,new_cc,old_cc))
				break;
		}
		
		// migrate done, close both files and move new_cc to  old_cc
		new_cc.close();
		old_cc.close();
		bt::Delete(current_chunks);
		bt::Move(tmp,current_chunks);
	}
	
	void MigrateCurrentChunks(const Torrent & tor,const QString & current_chunks)
	{
		try
		{
			MigrateCC(tor,current_chunks);
		}
		catch (...)
		{
			// cleanup tmp files upon error
			bt::Delete("/tmp/kt_current_chunks",true);
			throw;
		}
	}

}
