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
#include <libutil/file.h>
#include "chunkmanager.h"
#include "torrent.h"
#include <libutil/error.h>
#include "bitset.h"
#include "singlefilecache.h"
#include "multifilecache.h"
#include <libutil/log.h>
#include "globals.h"

#include <klocale.h>

namespace bt
{
	
	

	ChunkManager::ChunkManager(Torrent & tor,const QString & data_dir)
	: tor(tor),chunks(tor.getNumChunks())
	{
		num_in_mem = 0;
		if (tor.isMultiFile())
			cache = new MultiFileCache(tor,data_dir);
		else
			cache = new SingleFileCache(tor,data_dir);
		
		index_file = data_dir + "index";
		Uint32 tsize = tor.getFileLength();	// total size	
		Uint32 csize = tor.getChunkSize();	// chunk size
		Uint32 lsize = tsize - (csize * (tor.getNumChunks() - 1)); // size of last chunk
		
		for (unsigned int i = 0;i < tor.getNumChunks();i++)
		{
			if (i + 1 < tor.getNumChunks())
				chunks.insert(i,new Chunk(i,csize));
			else
				chunks.insert(i,new Chunk(i,lsize));
		}
		chunks.setAutoDelete(true);
		num_chunks_in_cache_file = 0;
		max_allowed = 50;
	}


	ChunkManager::~ChunkManager()
	{
		delete cache;
	}

	void ChunkManager::changeDataDir(const QString & data_dir)
	{
		cache->changeDataDir(data_dir);
		index_file = data_dir + "index";
	}
	
	void ChunkManager::loadIndexFile()
	{
		File fptr;
		if (!fptr.open(index_file,"rb"))
			throw Error(i18n("Can't open index file"));

		if (fptr.seek(File::END,0) == 0)
			return;

		fptr.seek(File::BEGIN,0);
		
		while (!fptr.eof())
		{
			NewChunkHeader hdr;
			fptr.read(&hdr,sizeof(NewChunkHeader));
			Chunk* c = getChunk(hdr.index);
			if (c)
			{
				max_allowed = hdr.index + 50;
				c->setStatus(Chunk::ON_DISK);
				c->setCacheFileOffset(hdr.cache_off);
			}
		}
	}
	
	void ChunkManager::saveIndexFile()
	{
		File fptr;
		if (!fptr.open(index_file,"wb"))
			throw Error(i18n("Can't open index file"));
		
		for (unsigned int i = 0;i < tor.getNumChunks();i++)
		{
			Chunk* c = getChunk(i);
			if (c->getStatus() != Chunk::NOT_DOWNLOADED)
			{
				NewChunkHeader hdr;
				hdr.cache_off = c->getCacheFileOffset();
				hdr.index = i;
				fptr.write(&hdr,sizeof(NewChunkHeader));
			}
		}
	}

	void ChunkManager::createFiles()
	{
		File fptr;
		fptr.open(index_file,"wb");
		cache->create();
	}

	Chunk* ChunkManager::getChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return 0;
		
		return chunks[i];
	}
	
	Chunk* ChunkManager::grabChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return 0;
		
		Chunk* c = chunks[i];
		if (c->getStatus() == Chunk::NOT_DOWNLOADED)
			return 0;

		if (c->getStatus() != Chunk::IN_MEMORY)
		{
			cache->load(c);
			num_in_mem++;
		}
		
		return c;
	}
		
	void ChunkManager::releaseChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;
		
		Chunk* c = chunks[i];
		c->unref();
	/*	if (!c->taken())
		{
			num_in_mem--;
			c->clear();
		}*/
	}
	
	void ChunkManager::saveChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;

		Chunk* c = chunks[i];
		cache->save(c);
		num_chunks_in_cache_file++;

		// update the index file
		writeIndexFileEntry(c);
	}

	void ChunkManager::writeIndexFileEntry(Chunk* c)
	{
		File fptr; 
		if (!fptr.open(index_file,"r+b"))
			throw Error(i18n("Can't open index file"));
		
		fptr.seek(File::END,0);
		NewChunkHeader hdr;
		hdr.cache_off = c->getCacheFileOffset();
		hdr.index = c->getIndex();
		fptr.write(&hdr,sizeof(NewChunkHeader));
		if (c->getIndex() + 50 > max_allowed)
			max_allowed = c->getIndex() + 50;
	}
	
	Uint32 ChunkManager::bytesLeft() const
	{
		Uint32 total = 0;
		for (Uint32 i = 0;i < chunks.size();i++)
		{
			const Chunk* c = chunks[i];
			if (c->getStatus() == Chunk::NOT_DOWNLOADED && !c->isExcluded())
				total += c->getSize();
		}
		return total;
	}
	
	void ChunkManager::toBitSet(BitSet & bs)
	{
		BitSet bits(chunks.size());
		for (Uint32 i = 0;i < chunks.size();i++)
		{
			const Chunk* c = chunks[i];
			if (c->getStatus() == Chunk::NOT_DOWNLOADED)
				bits.set(i,false);
			else
				bits.set(i,true);
		}
		bs = bits;
	}
	
	Uint32 ChunkManager::chunksLeft() const
	{
		Uint32 num = 0;
		for (Uint32 i = 0;i < chunks.size();i++)
		{
			const Chunk* c = chunks[i];
			if (c->getStatus() == Chunk::NOT_DOWNLOADED && !c->isExcluded())
				num++;
		}
		return num;
	}
	
	void ChunkManager::save(const QString & dir)
	{
		if (chunksLeft() != 0)
			return;

		cache->saveData(dir);
	}

	bool ChunkManager::hasBeenSaved() const
	{
		return cache->hasBeenSaved();
	}
	
	void ChunkManager::debugPrintMemUsage()
	{
		Out() << "Active Chunks : " << num_in_mem << endl;
	}

	const Uint32 MAX_CHUNK_IN_MEM = 10;

	void ChunkManager::checkMemoryUsage()
	{
		if (num_in_mem <= MAX_CHUNK_IN_MEM)
			return;
		
		Out() << "Getting rid of unnecessary Chunks" << endl;
		// try to keep at most 10 Chunk's in memory
		for (Uint32 i = 0;i < chunks.count() && num_in_mem > MAX_CHUNK_IN_MEM;i++)
		{
			Chunk* c = chunks[i];
			if (c->getStatus() == Chunk::IN_MEMORY && !c->taken())
			{
				num_in_mem--;
				c->clear();
			}
		}
		
	}

	void ChunkManager::prioritise(Uint32 from,Uint32 to)
	{
		if (from > to)
			std::swap(from,to);

		Uint32 i = from;
		while (i <= to && i < chunks.count())
		{
			Chunk* c = chunks[i];
			c->setPriority(true);
			i++;
		}
	}

	void ChunkManager::exclude(Uint32 from,Uint32 to)
	{
		if (from > to)
			std::swap(from,to);

		Uint32 i = from;
		while (i <= to && i < chunks.count())
		{
			Chunk* c = chunks[i];
			c->setExclude(true);
			i++;
		}
	}

	void ChunkManager::include(Uint32 from,Uint32 to)
	{
		if (from > to)
			std::swap(from,to);

		Uint32 i = from;
		while (i <= to && i < chunks.count())
		{
			Chunk* c = chunks[i];
			c->setExclude(false);
			i++;
		}
	}
}

