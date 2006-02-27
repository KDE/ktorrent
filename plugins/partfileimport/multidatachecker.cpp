/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson & Maggioni Marcello               *
 *   joris.guisson@gmail.com                                               *
 *   marcello.maggioni@gmail.com                                           *
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
		
		Uint64 chunk_size = tor.getChunkSize();
		Uint32 NumOfFiles = 0;
		Uint32 CurrentChunk = 0;
		Uint32 ReadBytes = 0;
		prog->setTotalSteps(num_chunks);
		
		Array<Uint8> buf((Uint32)tor.getChunkSize());
		
		NumOfFiles = tor.getNumFiles();
		for (Uint32 CurrentFile = 0;CurrentFile < NumOfFiles;CurrentFile++)
		{
			
			const TorrentFile & FileToRead = tor.getFile(CurrentFile);
			CurrentChunk = FileToRead.getFirstChunk();
			ReadBytes = FileToRead.getFirstChunkOffset();
			File FilePointer;
			if (!FilePointer.open(cache + FileToRead.getPath(), "rb"))
			{
				Out() << QString("Warning : Cannot open %1 : %2").arg(cache + 
				FileToRead.getPath()).arg(FilePointer.errorString()) << endl;
			}
			else
			{	
				for ( ; CurrentChunk <= FileToRead.getLastChunk(); CurrentChunk++ )
				{	
					prog->setProgress(CurrentChunk);
				
					Out() << "Checked " << CurrentChunk << " chunks" << endl;
					KApplication::kApplication()->processEvents();

					ReadBytes += FilePointer.read(buf + ReadBytes, chunk_size - ReadBytes);
					if (ReadBytes == chunk_size || CurrentChunk + 1 == tor.getNumChunks() )
					{
						SHA1Hash ChunkHash = SHA1Hash::generate(buf, ReadBytes );
						bool ok = (ChunkHash == tor.getHash(CurrentChunk));
						downloaded.set(CurrentChunk,ok);
						failed.set(CurrentChunk,!ok);
						ReadBytes = 0;
					}
				}
			}
				
		}
	}
}
