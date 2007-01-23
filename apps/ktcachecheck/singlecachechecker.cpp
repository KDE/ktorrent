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
#include "singlecachechecker.h"

using namespace bt;

namespace ktdebug
{

	SingleCacheChecker::SingleCacheChecker(bt::Torrent& tor): CacheChecker(tor)
	{}


	SingleCacheChecker::~SingleCacheChecker()
	{}


	void SingleCacheChecker::check(const QString& cache, const QString& index)
	{
		loadIndex(index);
		Uint32 num_chunks = tor.getNumChunks();
		File fptr;
		if (!fptr.open(cache,"rb"))
		{
			throw Error(QString("Cannot open file : %1 : %2")
					.arg(cache).arg( fptr.errorString()));
		}

		Uint32 num_ok = 0,num_not_ok = 0,num_not_downloaded = 0,extra_ok = 0;

		Array<Uint8> buf((Uint32)tor.getChunkSize());
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			if (i % 100 == 0)
				Out(SYS_GEN|LOG_DEBUG) << "Checked " << i << " chunks" << endl;
	//	Out() << "Chunk " << i << " : ";
			if (!fptr.eof())
			{
				Uint32 size = i == num_chunks - 1 && tor.getFileLength() % tor.getChunkSize() > 0 ?
						tor.getFileLength() % tor.getChunkSize() : (Uint32)tor.getChunkSize();
				fptr.seek(File::BEGIN,(Int64)i*tor.getChunkSize());
				fptr.read(buf,size);
				SHA1Hash h = SHA1Hash::generate(buf,size);
				bool ok = (h == tor.getHash(i));
				if (ok)
				{
					if (downloaded_chunks.count(i) == 0)
					{
						extra_ok++;
						extra_chunks.insert(i);
						continue;
					}
					num_ok++;
		//		Out() << "OK" << endl;
				}
				else
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
			
			}
			else
			{
				num_not_downloaded++;
			//Out() << "Not Downloaded" << endl;
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
