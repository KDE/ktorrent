/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#include <algorithm>
#include <util/file.h>
#include "chunkmanager.h"
#include "torrent.h"
#include <util/error.h>
#include <util/bitset.h>
#include <util/fileops.h>
#include "singlefilecache.h"
#include "multifilecache.h"
#include <util/log.h>
#include "globals.h"

#include <klocale.h>

namespace bt
{
	
	

	ChunkManager::ChunkManager(Torrent & tor,const QString & tmpdir,const QString & datadir)
	: tor(tor),chunks(tor.getNumChunks()),
	bitset(tor.getNumChunks()),excluded_chunks(tor.getNumChunks())
	{
		if (tor.isMultiFile())
			cache = new MultiFileCache(tor,tmpdir,datadir);
		else
			cache = new SingleFileCache(tor,tmpdir,datadir);
		
		index_file = tmpdir + "index";
		file_info_file = tmpdir + "file_info";
		Uint64 tsize = tor.getFileLength();	// total size
		Uint64 csize = tor.getChunkSize();	// chunk size
		Uint64 lsize = tsize - (csize * (tor.getNumChunks() - 1)); // size of last chunk
		
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
		chunks_left = 0;
		recalc_chunks_left = true;

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
	
		if(tor.isMultiFile())
		{
			for(Uint32 i=0; i<tor.getNumFiles(); ++i)
			{
				bt::TorrentFile & file = tor.getFile(i);
				if(file.isMultimedia()) 
					prioritise(file.getFirstChunk(), file.getFirstChunk()+1);
			}
		}
		else
		{
			if(tor.isMultimedia() )
			{
				this->prioritise(0,1);
				//this->prioritise(getNumChunks()-2, getNumChunks()-1);
			}
		}
	}


	ChunkManager::~ChunkManager()
	{
		delete cache;
	}
	
	QString ChunkManager::getDataDir() const
	{
		return cache->getDataDir();
	}

	void ChunkManager::changeDataDir(const QString & data_dir)
	{
		QValueList<Uint32> mapped;
		// save all buffered and mapped chunks
		for (Uint32 i = 0;i < tor.getNumChunks();i++)
		{
			Chunk* c = getChunk(i);
			if (c->getStatus() == Chunk::MMAPPED ||
				c->getStatus() == Chunk::BUFFERED)
			{
				cache->save(c);
				mapped.append(i);
			}
		}
		cache->close();
		cache->changeTmpDir(data_dir);
		cache->open();
		// reload the previously mapped and buffered chunks
		for (Uint32 i = 0;i < mapped.count();i++)
		{
			Chunk* c = getChunk(mapped[i]);
			cache->load(c);
		}
		
		index_file = data_dir + "index";
		file_info_file = data_dir + "file_info";
		saveFileInfo();
	}
	
	void ChunkManager::loadIndexFile()
	{
		loadFileInfo();
		
		File fptr;
		if (!fptr.open(index_file,"rb"))
		{
			// no index file, so assume it's empty
			bt::Touch(index_file,true);
			Out() << "Can't open index file : " << fptr.errorString() << endl;
			return;
		}

		if (fptr.seek(File::END,0) != 0)
		{
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
					bitset.set(hdr.index,true);
					recalc_chunks_left = true;
				}
			}
		}
	}
	
	void ChunkManager::saveIndexFile()
	{
		File fptr;
		if (!fptr.open(index_file,"wb"))
			throw Error(i18n("Cannot open index file %1 : %2").arg(index_file).arg(fptr.errorString()));
		
		for (unsigned int i = 0;i < tor.getNumChunks();i++)
		{
			Chunk* c = getChunk(i);
			if (c->getStatus() != Chunk::NOT_DOWNLOADED)
			{
				NewChunkHeader hdr;
				hdr.index = i;
				fptr.write(&hdr,sizeof(NewChunkHeader));
			}
		}
		saveFileInfo();
	}

	void ChunkManager::createFiles()
	{
		if (!bt::Exists(index_file))
		{
			File fptr;
			fptr.open(index_file,"wb");
		}
		cache->create();
	}

	Chunk* ChunkManager::getChunk(unsigned int i)
	{
		if (i >= chunks.count())
			return 0;
		else
			return chunks[i];
	}
	
	void ChunkManager::start()
	{
		cache->open();
	}
		
	void ChunkManager::stop()
	{
		// unmmap all chunks which can
		for (Uint32 i = 0;i < bitset.getNumBits();i++)
		{
			Chunk* c = chunks[i];
			if (c->getStatus() == Chunk::MMAPPED)
			{
				cache->save(c);
				c->clear();
				c->setStatus(Chunk::ON_DISK);
			}
			else if (c->getStatus() == Chunk::BUFFERED)
			{
				c->clear();
				c->setStatus(Chunk::ON_DISK);
			}
		}
		cache->close();
	}
	
	Chunk* ChunkManager::grabChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return 0;
		
		Chunk* c = chunks[i];
		if (c->getStatus() == Chunk::NOT_DOWNLOADED)
		{
			return 0;
		}
		else if (c->getStatus() == Chunk::ON_DISK)
		{
			// load the chunk if it is on disk
			cache->load(c);
			loaded.append(i);
		}
		
		return c;
	}
		
	void ChunkManager::releaseChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;
		
		Chunk* c = chunks[i];
		if (!c->taken())
		{
			if (c->getStatus() == Chunk::MMAPPED)
				cache->save(c);
			c->clear();
			c->setStatus(Chunk::ON_DISK);
			loaded.remove(i);
		}
	}
	
	void ChunkManager::checkMemoryUsage()
	{
		Uint32 num_removed = 0;
		QValueList<Uint32>::iterator i = loaded.begin();
		while (i != loaded.end())
		{
			Chunk* c = chunks[*i];
			if (!c->taken())
			{
				if (c->getStatus() == Chunk::MMAPPED)
					cache->save(c);
				c->clear();
				c->setStatus(Chunk::ON_DISK);
				i = loaded.erase(i);
				num_removed++;
			}
			else
			{
				i++;
			}
		}
//		Uint32 num_in_mem = loaded.count();
		//Out() << QString("Cleaned %1 chunks, %2 still in memory").arg(num_removed).arg(num_in_mem) << endl;
	}
	
	void ChunkManager::saveChunk(unsigned int i,bool update_index)
	{
		if (i >= chunks.size())
			return;

		Chunk* c = chunks[i];
		cache->save(c);
		
		// update the index file
		if (update_index)
		{
			num_chunks_in_cache_file++;
			bitset.set(i,true);
			recalc_chunks_left = true;
			writeIndexFileEntry(c);
		}
	}

	void ChunkManager::writeIndexFileEntry(Chunk* c)
	{
		File fptr; 
		if (!fptr.open(index_file,"r+b"))
		{
			// no index file, so assume it's empty
			bt::Touch(index_file,true);
			Out() << "Can't open index file : " << fptr.errorString() << endl;
			// try again
			if (!fptr.open(index_file,"r+b"))
				// panick if it failes
				throw Error(i18n("Cannot open index file %1 : %2").arg(index_file).arg(fptr.errorString()));
		}

		
		fptr.seek(File::END,0);
		NewChunkHeader hdr;
		hdr.index = c->getIndex();
		fptr.write(&hdr,sizeof(NewChunkHeader));
		if (c->getIndex() + 50 > max_allowed)
			max_allowed = c->getIndex() + 50;
	}
	
	Uint64 ChunkManager::bytesLeft() const
	{
		Uint32 num_left = chunksLeft();
		Uint32 last = chunks.size() - 1;
		if (!bitset.get(last) && !excluded_chunks.get(last))
		{
			Chunk* c = chunks[last];
			return (num_left - 1)*tor.getChunkSize() + c->getSize();
		}
		else
		{
			return num_left*tor.getChunkSize();
		}
	}
	
	Uint32 ChunkManager::chunksLeft() const
	{
		if (!recalc_chunks_left)
			return chunks_left;
		
		Uint32 num = 0;
		Uint32 tot = chunks.size();
		for (Uint32 i = 0;i < tot;i++)
		{
			const Chunk* c = chunks[i];
			if (!bitset.get(i) && !c->isExcluded())
				num++;
		}
		chunks_left = num;
		recalc_chunks_left = false;
		return num;
	}

	Uint64 ChunkManager::bytesExcluded() const
	{
		if (excluded_chunks.get(tor.getNumChunks() - 1))
		{
			Chunk* c = chunks[tor.getNumChunks() - 1];
			Uint32 num = excluded_chunks.numOnBits() - 1;
			return tor.getChunkSize() * num + c->getSize();
		}
		else
		{
			return tor.getChunkSize() * excluded_chunks.numOnBits();
		}
	}

	Uint32 ChunkManager::chunksExcluded() const
	{
		return excluded_chunks.numOnBits();
	}
	
	void ChunkManager::debugPrintMemUsage()
	{
		Out() << "Active Chunks : " << loaded.count()<< endl;
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
			excluded_chunks.set(i,true);
			i++;
		}
		recalc_chunks_left = true;
		saveFileInfo();
		excluded(from,to);
		updateStats();
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
			excluded_chunks.set(i,false);
			i++;
		}
		recalc_chunks_left = true;
		saveFileInfo();
		updateStats();
	}

	void ChunkManager::saveFileInfo()
	{
		// saves which TorrentFile's do not need to be downloaded
		File fptr;
		if (!fptr.open(file_info_file,"wb"))
		{
			Out() << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

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
		fptr.flush();
	}
	
	void ChunkManager::loadFileInfo()
	{
		File fptr;
		if (!fptr.open(file_info_file,"rb"))
			return;

		Uint32 num = 0,tmp = 0;
		// first read the number of dnd files
		if (fptr.read(&num,sizeof(Uint32)) != sizeof(Uint32))
		{
			Out() << "Warning : error reading chunk_info file" << endl;
			return;
		}

		for (Uint32 i = 0;i < num;i++)
		{
			if (fptr.read(&tmp,sizeof(Uint32)) != sizeof(Uint32))
			{
				Out() << "Warning : error reading chunk_info file" << endl;
				return;
			}

			bt::TorrentFile & tf = tor.getFile(tmp);
			if (!tf.isNull())
			{
				Out() << "Excluding : " << tf.getPath() << endl;
				tf.setDoNotDownload(true);
			}
		}
	}

	void ChunkManager::downloadStatusChanged(TorrentFile* tf,bool download)
	{
		if (download)
		{
			// include the range of the file
			include(tf->getFirstChunk(),tf->getLastChunk());
		}
		else
		{
			Uint32 first = tf->getFirstChunk();
			Uint32 last = tf->getLastChunk();

			// first and last chunk may be part of multiple files
			// so we can't just exclude them
			QValueList<Uint32> files;

			// get list of files where first chunk lies in
			tor.calcChunkPos(first,files);
			// check for exceptional case which causes very long loops
			if (first == last && files.count() > 1)
				return;
			
			// if one file in the list needs to be downloaded,increment first
			for (QValueList<Uint32>::iterator i = files.begin();i != files.end();i++)
			{
				if (!tor.getFile(*i).doNotDownload())
				{
					first++;
					break;
				}
			}
			
			files.clear();
			// get list of files where last chunk lies in
			tor.calcChunkPos(last,files);
			// if one file in the list needs to be downloaded,decrement last
			for (QValueList<Uint32>::iterator i = files.begin();i != files.end();i++)
			{
				if (!tor.getFile(*i).doNotDownload())
				{
					last--;
					break;
				}
			}

			// last smaller then first is not normal, so just return
			if (last < first)
				return;
			
			exclude(first,last);
			
			
		}
		// alert the cache but first put things in critical operation mode
		cache->downloadStatusChanged(tf,download);
	}
	
	bool ChunkManager::prepareChunk(Chunk* c)
	{
		if (c->getStatus() != Chunk::NOT_DOWNLOADED)
			return false;
		
		return cache->prep(c);
	}
}

#include "chunkmanager.moc"
