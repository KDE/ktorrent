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

#include <klocale.h>
#include <kprogress.h>
#include <kapplication.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentfile.h>
#include "multidatachecker.h"

namespace bt
{

	MultiDataChecker::MultiDataChecker(): DataChecker()
	{}


	MultiDataChecker::~MultiDataChecker()
	{}


	void MultiDataChecker::check(const QString& path, const Torrent& tor,KProgress* prog)
	{
		Uint32 num_chunks = tor.getNumChunks();
		// initialize the bitsets
		downloaded = BitSet(num_chunks);
		failed = BitSet(num_chunks);
		
		QString cache = path;
		if (!cache.endsWith(bt::DirSeparator()))
			cache += bt::DirSeparator();
		
		Uint64 curr_file_off = 0;
		Uint32 curr_file = 0;
		Uint64 chunk_size = tor.getChunkSize();

		prog->setTotalSteps(num_chunks);
		
		Array<Uint8> buf((Uint32)tor.getChunkSize());
		
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			prog->setProgress(i);
			if (i % 50 == 0 && i > 0)
			{
				Out() << "Checked " << i << " chunks" << endl;
				KApplication::kApplication()->processEvents();
			}
	
			Uint64 size = chunk_size;
			if (i == tor.getNumChunks() - 1 && tor.getFileLength() % chunk_size != 0)
				size = tor.getFileLength() % chunk_size;

			//Out() << "Loading chunk (size = " << size << ")" << endl;
			Uint64 bytes_offset = 0;
			while (bytes_offset < size)
			{
				const TorrentFile & tf = tor.getFile(curr_file);
// 				Out() << "Current file : " << tf.getPath() << " (" << curr_file << ")" << endl;
				Uint64 to_read = size - bytes_offset;
// 				Out() << "to_read = " << to_read << endl;
				if (to_read <= tf.getSize() - curr_file_off)
				{
					// we can read the chunk from this file
					File fptr;
					if (!fptr.open(cache + tf.getPath(),"rb"))
						throw Error(i18n("Cannot open %1 : %2").arg(cache + tf.getPath()).arg(fptr.errorString()));

					fptr.seek(File::BEGIN,curr_file_off);
					fptr.read(buf + bytes_offset,to_read);
					bytes_offset += to_read;
					curr_file_off += to_read;
				}
				else
				{
					
					// read partially the data which can be read
					to_read = tf.getSize() - curr_file_off;
// 					Out() << "Partially reading " << to_read << endl;
					File fptr;
					if (!fptr.open(cache + tf.getPath(),"rb"))
						throw Error(i18n("Cannot open %1 : %2").arg(cache + tf.getPath()).arg(fptr.errorString()));

					fptr.seek(File::BEGIN,curr_file_off);
					fptr.read(buf + bytes_offset,to_read);
					bytes_offset += to_read;
					// update curr_file and offset
					curr_file++;
					curr_file_off = 0;
				}
			} // end file reading while

			// calculate hash and check it
			SHA1Hash h = SHA1Hash::generate(buf,size);
			bool ok = h == tor.getHash(i);
			downloaded.set(i,ok);
			failed.set(i,!ok);
		}
	}

}
