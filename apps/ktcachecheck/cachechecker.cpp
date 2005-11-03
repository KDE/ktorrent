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
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/chunkmanager.h>
#include "cachechecker.h"

using namespace bt;

namespace debug
{

	CacheChecker::CacheChecker(bt::Torrent & tor) : tor(tor)
	{}


	CacheChecker::~CacheChecker()
	{}

	void CacheChecker::loadIndex(const QString & index_file)
	{
		this->index_file = index_file;
		File fptr;
		if (!fptr.open(index_file,"rb"))
			throw Error("Can't open index file : " + fptr.errorString());

		if (fptr.seek(File::END,0) != 0)
		{
			fptr.seek(File::BEGIN,0);
			
			while (!fptr.eof())
			{
				NewChunkHeader hdr;
				fptr.read(&hdr,sizeof(NewChunkHeader));
				downloaded_chunks.insert(hdr.index);
			}
		}
	}

	struct ChunkHeader
	{
		unsigned int index; // the Chunks index
		unsigned int deprecated; // offset in cache file
	};
	

	void CacheChecker::fixIndex()
	{
		if (failed_chunks.size() == 0)
			return;

		File fptr;
		if (!fptr.open(index_file,"wb"))
			throw Error("Can't open index file : " + fptr.errorString());

		// first remove failed chunks from downloaded
		std::set<bt::Uint32>::iterator i = failed_chunks.begin();
		while (i != failed_chunks.end())
		{
			downloaded_chunks.erase(*i);
			i++;
		}

		// write remaining chunks
		i = downloaded_chunks.begin();
		while (i != downloaded_chunks.end())
		{
			ChunkHeader ch = {*i,0};
			fptr.write(&ch,sizeof(ChunkHeader));
			i++;
		}
	}
}
