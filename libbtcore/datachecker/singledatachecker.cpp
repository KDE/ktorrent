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
#include "singledatachecker.h"
#include <klocale.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>

namespace bt
{

	SingleDataChecker::SingleDataChecker(): DataChecker()
	{}


	SingleDataChecker::~SingleDataChecker()
	{}


	void SingleDataChecker::check(const QString& path, const Torrent& tor,const QString &,const BitSet & status)
	{
		// open the file
		Uint32 num_chunks = tor.getNumChunks();
		Uint32 chunk_size = tor.getChunkSize();
		File fptr;
		if (!fptr.open(path,"rb"))
		{
			throw Error(i18n("Cannot open file : %1 : %2", path, fptr.errorString()));
		}

		// initialize the bitset
		result = BitSet(num_chunks);
		// loop over all chunks
		Array<Uint8> buf(chunk_size);
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			if (listener)
			{
				listener->progress(i,num_chunks);
				if (listener->needToStop()) // if we need to stop just return
					return;
			}
	
			if (!fptr.eof())
			{
				// read the chunk
				Uint32 size = i == num_chunks - 1 ? tor.getLastChunkSize() : tor.getChunkSize();
				
				fptr.seek(File::BEGIN,(Int64)i*tor.getChunkSize());
				fptr.read(buf,size);
				// generate and test hash
				SHA1Hash h = SHA1Hash::generate(buf,size);
				bool ok = (h == tor.getHash(i));
				result.set(i,ok);
				if (ok && status.get(i))
					downloaded++;
				else if (!ok && status.get(i))
					failed++;
				else if (!ok && !status.get(i))
					not_downloaded++;
				else if (ok && !status.get(i))
					found++;
			}
			else
			{
				// at end of file so set to default values for a failed chunk
				result.set(i,false);
				if (!status.get(i))
					not_downloaded++;
				else
					failed++;
			}
			if (listener)
				listener->status(failed,found,downloaded,not_downloaded);
		}
	}

}
