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
#include <kapplication.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include "singledatachecker.h"

namespace bt
{

	SingleDataChecker::SingleDataChecker(): DataChecker()
	{}


	SingleDataChecker::~SingleDataChecker()
	{}


	void SingleDataChecker::check(const QString& path, const Torrent& tor,const QString &)
	{
		// open the file
		Uint32 num_chunks = tor.getNumChunks();
		Uint32 chunk_size = tor.getChunkSize();
		File fptr;
		if (!fptr.open(path,"rb"))
		{
			throw Error(i18n("Cannot open file : %1 : %2")
					.arg(path).arg( fptr.errorString()));
		}

		// initialize the bitsets
		downloaded = BitSet(num_chunks);
		failed = BitSet(num_chunks);
		
		TimeStamp last_update_time = bt::GetCurrentTime();
		
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
			
			TimeStamp now = bt::GetCurrentTime();
			if (now - last_update_time > 1000)
			{
				Out(SYS_DIO|LOG_DEBUG) << "Checked " << i << " chunks" << endl;
				last_update_time = now;
			}
	
			if (!fptr.eof())
			{
				// read the chunk
				Uint32 size = i == num_chunks - 1 && tor.getFileLength() % tor.getChunkSize() > 0 ?
						tor.getFileLength() % tor.getChunkSize() : (Uint32)tor.getChunkSize();
				
				fptr.seek(File::BEGIN,(Int64)i*tor.getChunkSize());
				fptr.read(buf,size);
				// generate and test hash
				SHA1Hash h = SHA1Hash::generate(buf,size);
				bool ok = (h == tor.getHash(i));
				downloaded.set(i,ok);
				failed.set(i,!ok);
			}
			else
			{
				// at end of file so set to default values for a failed chunk
				downloaded.set(i,false);
				failed.set(i,true);
			}
			if (listener)
				listener->status(failed.numOnBits(),downloaded.numOnBits());
		}
	}

}
