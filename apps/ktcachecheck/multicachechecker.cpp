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
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/chunkmanager.h>
#include "multicachechecker.h"

using namespace bt;

namespace ktdebug
{

	MultiCacheChecker::MultiCacheChecker(bt::Torrent& tor): CacheChecker(tor)
	{}


	MultiCacheChecker::~MultiCacheChecker()
	{}


	void MultiCacheChecker::check(const QString& cache_dir, const QString& index)
	{
		loadIndex(index);
		QString cache = cache_dir;
		if (!cache.endsWith(bt::DirSeparator()))
			cache += bt::DirSeparator();

		Uint32 num_chunks = tor.getNumChunks();
		Uint64 curr_file_off = 0;
		Uint32 curr_file = 0;
		Uint64 chunk_size = tor.getChunkSize();

		Array<Uint8> buf((Uint32)tor.getChunkSize());
		Uint32 num_ok = 0,num_not_ok = 0,num_not_downloaded = 0,extra_ok = 0;
	
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			if (i % 100 == 0)
				Out() << "Checked " << i << " chunks" << endl;


			
			Uint64 size = chunk_size;
			if (i == tor.getNumChunks() - 1 && tor.getFileLength() % chunk_size != 0)
				size = tor.getFileLength() % chunk_size;

			//Out() << "Loading chunk (size = " << size << ")" << endl;
			Uint64 bytes_offset = 0;
			while (bytes_offset < size)
			{
				TorrentFile & tf = tor.getFile(curr_file);
// 				Out() << "Current file : " << tf.getPath() << " (" << curr_file << ")" << endl;
				Uint64 to_read = size - bytes_offset;
// 				Out() << "to_read = " << to_read << endl;
				if (to_read <= tf.getSize() - curr_file_off)
				{
					// we can read the chunk from this file
					File fptr;
					if (!fptr.open(cache + tf.getPath(),"rb"))
						throw Error(QString("Cannot open %1 : %2").arg(cache + tf.getPath()).arg(fptr.errorString()));

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
						throw Error(QString("Cannot open %1 : %2").arg(cache + tf.getPath()).arg(fptr.errorString()));

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
			if (h != tor.getHash(i))
			{
				if (downloaded_chunks.count(i) == 0)
				{
					num_not_downloaded++;
					continue;
				}
				Out() << "Chunk " << i << " Failed :" << endl;
				Out() << "\tShould be : " << tor.getHash(i).toString() << endl;
				Out() << "\tIs        : " << h.toString() << endl;
				num_not_ok++;
				failed_chunks.insert(i);
			}
			else
			{
				if (downloaded_chunks.count(i) == 0)
				{
					extra_ok++;
					extra_chunks.insert(i);
					continue;
				}
				num_ok++;
			}
		}

		Out() << "Cache Check Summary" << endl;
		Out() << "===================" << endl;
		Out() << "Extra Chunks : " << extra_ok << endl;
		Out() << "Chunks OK : " << num_ok << endl;
		Out() << "Chunks Not Downloaded : " << num_not_downloaded << endl;
		Out() << "Chunks Failed : " << num_not_ok << endl;
	}

}
