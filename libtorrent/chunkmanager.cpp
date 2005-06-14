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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "file.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "error.h"
#include "bitset.h"

namespace bt
{
	
	struct NewChunkHeader
	{
		unsigned int index; // the Chunks index
		unsigned int cache_off; // offset in cache file
	};

	ChunkManager::ChunkManager(Torrent & tor,const QString & data_dir)
	: tor(tor),chunks(tor.getNumChunks())
	{
		cache_file = data_dir + "cache";
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

	}

	void ChunkManager::changeDataDir(const QString & data_dir)
	{
		cache_file = data_dir + "cache";
		index_file = data_dir + "index";
	}
	
	void ChunkManager::loadIndexFile()
	{
		File fptr;
		if (!fptr.open(index_file,"rb"))
			throw Error("Can't open index file");

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
			throw Error("Can't open index file");
		
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
		fptr.open(cache_file,"wb");
		fptr.open(index_file,"wb");
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
			loadChunk(i);
		
		c->ref();
		return c;
	}
	
	void ChunkManager::loadChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;
		
		Chunk* c = chunks[i];
		if (c->getStatus() == Chunk::NOT_DOWNLOADED)
			return;
		
		File fptr;
		if (!fptr.open(cache_file,"rb"))
			throw Error("Can't open cache file");
		fptr.seek(File::BEGIN,c->getCacheFileOffset());
		unsigned char* data = new unsigned char[c->getSize()];
		fptr.read(data,c->getSize());
		c->setData(data);
	}
	
	void ChunkManager::releaseChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;
		
		Chunk* c = chunks[i];
		c->unref();
		if (!c->taken())
			c->clear();
	}
	
	void ChunkManager::saveChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;

		Chunk* c = chunks[i];
		// calculate the chunks position in the final file
		Uint32 chunk_pos = i * tor.getChunkSize();
		
		File fptr;
		if (!fptr.open(cache_file,"r+b"))
			throw Error("Can't open cache file");
		
		// jump to end of file
		fptr.seek(File::END,0);
		unsigned int cache_size = fptr.tell();

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
			Uint32 num_empty_bytes = chunk_pos - cache_size;
			Uint8 b[1024];
			Uint32 nw = 0;
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
		
		// set the offset and clear the chunk
		c->setCacheFileOffset(chunk_pos);
		c->clear();
		
		num_chunks_in_cache_file++;

		// update the index file
		writeIndexFileEntry(c);
	}

	void ChunkManager::writeIndexFileEntry(Chunk* c)
	{
		File fptr; 
		if (!fptr.open(index_file,"r+b"))
			throw Error("Can't open index file");
		
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
			if (c->getStatus() == Chunk::NOT_DOWNLOADED)
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
			if (c->getStatus() == Chunk::NOT_DOWNLOADED)
				num++;
		}
		return num;
	}
}

