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
#include <qfileinfo.h>
#include <util/fileops.h>
#include "torrent.h"
#include "chunk.h"
#include <util/file.h>
#include "singlefilecache.h"
#include <util/error.h>
#include <util/functions.h>

#include <klocale.h>

namespace bt
{

	SingleFileCache::SingleFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir)
	: Cache(tor,tmpdir,datadir)
	{
		cache_file = tmpdir + "cache";
	}


	SingleFileCache::~SingleFileCache()
	{}

	void SingleFileCache::changeDataDir(const QString & ndir)
	{
		Cache::changeTmpDir(ndir);
		cache_file = tmpdir + "cache";
	}

	void SingleFileCache::load(Chunk* c)
	{
		File fptr;
		if (!fptr.open(cache_file,"rb"))
			throw Error(i18n("Unable to open cache file: %1").arg(fptr.errorString()));
		Uint64 off = c->getIndex() * tor.getChunkSize();
		fptr.seek(File::BEGIN,off);
		unsigned char* data = new unsigned char[c->getSize()];
		fptr.read(data,c->getSize());
		c->setData(data);
	}

	void SingleFileCache::save(Chunk* c)
	{
		// calculate the chunks position in the final file
		Uint64 chunk_pos = c->getIndex() * tor.getChunkSize();
		
		File fptr;
		if (!fptr.open(cache_file,"r+b"))
			throw Error(i18n("Unable to open cache file: %1").arg(fptr.errorString()));
		
		// jump to end of file
		fptr.seek(File::END,0);
		Uint64 cache_size = fptr.tell();

		// see if the cache is big enough for the chunk
		if (chunk_pos <= cache_size)
		{
			// big enough so just jump to the right posiition and write
			// the chunk
			fptr.seek(File::BEGIN,chunk_pos);
			fptr.write(c->getData(),c->getSize());
		}
		else
		{
			// write random shit to enlarge the file
			Uint64 num_empty_bytes = chunk_pos - cache_size;
			Uint8 b[1024];
			Uint64 nw = 0;
			while (nw < num_empty_bytes)
			{
				Uint32 left = num_empty_bytes - nw;
				fptr.write(b,left < 1024 ? left : 1024);
				nw += 1024;
			}

			// now write the chunks at the good position
			fptr.seek(File::BEGIN,chunk_pos);
			fptr.write(c->getData(),c->getSize());
		}
		
		// clear the chunk
		c->clear();
	}

	void SingleFileCache::create()
	{
		if (!bt::Exists(datadir + tor.getNameSuggestion()))
		{
			QString out_file = datadir + tor.getNameSuggestion();
			bt::Touch(out_file);
			bt::SymLink(out_file,cache_file);
		}
	}

}
