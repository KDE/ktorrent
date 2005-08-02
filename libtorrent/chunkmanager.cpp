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
		chunk_info_file = data_dir + "chunk_info";
		Uint32 tsize = tor.getFileLength();	// total size	
		Uint32 csize = tor.getChunkSize();	// chunk size
		Uint32 lsize = tsize - (csize * (tor.getNumChunks() - 1)); // size of last chunk
		
		for (Uint32 i = 0;i < tor.getNumChunks();i++)
		{
			if (i + 1 < tor.getNumChunks())
				chunks.insert(i,new Chunk(i,csize));
			else
				chunks.insert(i,new Chunk(i,lsize));
		}
		chunks.setAutoDelete(true);
		num_chunks_in_cache_file = 0;
		max_allowed = 50;

		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			connect(&tf,SIGNAL(downloadStatusChanged(TorrentFile*, bool )),
					 this,SLOT(downloadStatusChanged(TorrentFile*, bool )));
			if (tf.doNotDownload())
			{
				downloadStatusChanged(&tf,false);
			}
		}
	}


	ChunkManager::~ChunkManager()
	{
		delete cache;
	}

	void ChunkManager::changeDataDir(const QString & data_dir)
	{
		cache->changeDataDir(data_dir);
		index_file = data_dir + "index";
		chunk_info_file = data_dir + "chunk_info";
		saveChunkInfo();
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

		loadChunkInfo();
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

	Uint32 ChunkManager::bytesExcluded() const
	{
		Uint32 num = 0;
		for (Uint32 i = 0;i < chunks.size();i++)
		{
			const Chunk* c = chunks[i];
			if (c->isExcluded())
				num += c->getSize();
		}
		return num;
	}

	Uint32 ChunkManager::chunksExcluded() const
	{
		Uint32 num = 0;
		for (Uint32 i = 0;i < chunks.size();i++)
		{
			const Chunk* c = chunks[i];
			if (c->isExcluded())
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
		saveChunkInfo();
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
		saveChunkInfo();
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
		saveChunkInfo();
	}

	void ChunkManager::saveChunkInfo()
	{
		// saves which TorrentFile's do not need to be downloaded
		File fptr;
		if (!fptr.open(chunk_info_file,"wb"))
			throw Error(i18n("Can't save chunk_info file : %1").arg(fptr.errorString()));

		QValueList<Uint32> dnd;
		
		Uint32 i = 0;
		while (i < tor.getNumFiles())
		{
			if (tor.getFile(i).doNotDownload())
				dnd.append(i);
			i++;
		}

		// first write the number of excluded ones
		Uint32 tmp = dnd.count();
		fptr.write(&tmp,sizeof(Uint32));
		// then all the excluded ones
		for (i = 0;i < dnd.count();i++)
		{
			tmp = dnd[i];
			fptr.write(&tmp,sizeof(Uint32));
		}
	}
	
	void ChunkManager::loadChunkInfo()
	{
		File fptr;
		if (!fptr.open(chunk_info_file,"rb"))
			return;

		Uint32 num = 0,tmp = 0;
		// first read the number of dnd files
		if (fptr.read(&num,sizeof(Uint32)) != sizeof(Uint32))
		{
			Out() << "Warning : error reading chunk_info file" << endl;
			return;
		}

		for (Uint32 i = 0;i < tmp;i++)
		{
			if (fptr.read(&tmp,sizeof(Uint32)) != sizeof(Uint32))
			{
				Out() << "Warning : error reading chunk_info file" << endl;
				return;
			}

			tor.getFile(i).setDoNotDownload(true);
		}
	}

	void ChunkManager::downloadStatusChanged(TorrentFile* tf,bool download)
	{
		if (download)
		{
			include(tf->getFirstChunk(),tf->getLastChunk());
		}
		else
		{
			Uint32 first = tf->getFirstChunk();
			Uint32 last = tf->getLastChunk();
			if (first != 0)
				first++;
			if (last != chunks.count() - 1 && last > 0)
				last--;

			if (last <= first)
				return;

			exclude(first,last);
		}
	}
}

#include "chunkmanager.moc"
