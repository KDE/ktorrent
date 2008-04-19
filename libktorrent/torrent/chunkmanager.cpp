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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <algorithm>
#include <util/file.h>
#include <util/array.h>
#include <qstringlist.h>
#include "chunkmanager.h"
#include "torrent.h"
#include <util/error.h>
#include <util/bitset.h>
#include <util/fileops.h>
#include "singlefilecache.h"
#include "multifilecache.h"
#include <util/log.h>
#include <util/functions.h>
#include "globals.h"

#include <klocale.h>

namespace bt
{
	
	Uint32 ChunkManager::max_chunk_size_for_data_check = 0;
	

	ChunkManager::ChunkManager(
			Torrent & tor,
			const QString & tmpdir,
			const QString & datadir,
			bool custom_output_name)
	: tor(tor),chunks(tor.getNumChunks()),
	bitset(tor.getNumChunks()),excluded_chunks(tor.getNumChunks()),only_seed_chunks(tor.getNumChunks()),todo(tor.getNumChunks())
	{
		during_load = false;
		only_seed_chunks.setAll(false);
		todo.setAll(true);
		if (tor.isMultiFile())
			cache = new MultiFileCache(tor,tmpdir,datadir,custom_output_name);
		else
			cache = new SingleFileCache(tor,tmpdir,datadir);
		
		index_file = tmpdir + "index";
		file_info_file = tmpdir + "file_info";
		file_priority_file = tmpdir + "file_priority";
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
		chunks_left = 0;
		recalc_chunks_left = true;
		corrupted_count = recheck_counter = 0;

		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			connect(&tf,SIGNAL(downloadPriorityChanged(TorrentFile*, Priority, Priority )),
					 this,SLOT(downloadPriorityChanged(TorrentFile*, Priority, Priority )));
			
			if (tf.getPriority() != NORMAL_PRIORITY)
			{
				downloadPriorityChanged(&tf,tf.getPriority(),tf.getOldPriority());
			}
		}
	
		if(tor.isMultiFile())
		{
			for(Uint32 i=0; i<tor.getNumFiles(); ++i)
			{
				bt::TorrentFile & file = tor.getFile(i);
				if (!file.isMultimedia() || file.getPriority() == bt::ONLY_SEED_PRIORITY) 
					continue;

				if (file.getFirstChunk() == file.getLastChunk())
				{
					// prioritise whole file 
					prioritise(file.getFirstChunk(),file.getLastChunk(),PREVIEW_PRIORITY);
				}
				else
				{
					Uint32 chunkOffset;
					chunkOffset = ((file.getLastChunk() - file.getFirstChunk()) / 100) + 1;
					prioritise(file.getFirstChunk(), file.getFirstChunk()+chunkOffset, PREVIEW_PRIORITY);
					if (file.getLastChunk() - file.getFirstChunk() > chunkOffset)
					{
						prioritise(file.getLastChunk() - chunkOffset, file.getLastChunk(), PREVIEW_PRIORITY);
					}
				}
			}
		}
		else
		{
			if(tor.isMultimedia())
			{
				Uint32 chunkOffset;
				chunkOffset = (tor.getNumChunks() / 100) + 1;

				prioritise(0,chunkOffset,PREVIEW_PRIORITY);
				if (tor.getNumChunks() > chunkOffset)
				{
					prioritise(tor.getNumChunks() - chunkOffset, tor.getNumChunks() - 1,PREVIEW_PRIORITY);
				}
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
		cache->changeTmpDir(data_dir);
		index_file = data_dir + "index";
		file_info_file = data_dir + "file_info";
		file_priority_file = data_dir + "file_priority";
	}
	
	KIO::Job* ChunkManager::moveDataFiles(const QString & ndir)
	{
		return cache->moveDataFiles(ndir);
	}
	
	void ChunkManager::moveDataFilesCompleted(KIO::Job* job)
	{
		cache->moveDataFilesCompleted(job);
	}
	
	void ChunkManager::changeOutputPath(const QString & output_path)
	{
		cache->changeOutputPath(output_path);
	}
	
	void ChunkManager::loadIndexFile()
	{
		during_load = true;
		loadPriorityInfo();
		
		File fptr;
		if (!fptr.open(index_file,"rb"))
		{
			// no index file, so assume it's empty
			bt::Touch(index_file,true);
			Out(SYS_DIO|LOG_IMPORTANT) << "Can't open index file : " << fptr.errorString() << endl;
			during_load = false;
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
					c->setStatus(Chunk::ON_DISK);
					bitset.set(hdr.index,true);
					todo.set(hdr.index,false);
					recalc_chunks_left = true;
				}
			}
		}
		tor.updateFilePercentage(bitset);
		during_load = false;
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
		savePriorityInfo();
	}

	void ChunkManager::createFiles(bool check_priority)
	{
		if (!bt::Exists(index_file))
		{
			File fptr;
			fptr.open(index_file,"wb");
		}
		cache->create();
		if (check_priority)
		{
			for (Uint32 i = 0;i < tor.getNumFiles();i++)
			{
				TorrentFile & tf = tor.getFile(i);
				connect(&tf,SIGNAL(downloadPriorityChanged(TorrentFile*, Priority, Priority )),
						this,SLOT(downloadPriorityChanged(TorrentFile*, Priority, Priority )));
				
				if (tf.getPriority() != NORMAL_PRIORITY)
				{
					downloadPriorityChanged(&tf,tf.getPriority(),tf.getOldPriority());
				}
			}
		}
	}
	
	bool ChunkManager::hasMissingFiles(QStringList & sl)
	{
		return cache->hasMissingFiles(sl);
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
		if (c->getStatus() == Chunk::NOT_DOWNLOADED || c->isExcluded())
		{
			return 0;
		}
		else if (c->getStatus() == Chunk::ON_DISK)
		{
			// load the chunk if it is on disk
			cache->load(c);
			loaded.insert(i,bt::GetCurrentTime());
			bool check_allowed = (max_chunk_size_for_data_check == 0 || tor.getChunkSize() <= max_chunk_size_for_data_check);
			
			// when no corruptions have been found, only check once every 5 chunks
			if (check_allowed && recheck_counter < 5 && corrupted_count == 0)
				check_allowed = false; 
			 
			if (c->getData() && check_allowed)
			{
				recheck_counter = 0;
				if (!c->checkHash(tor.getHash(i)))
				{
					Out(SYS_DIO|LOG_IMPORTANT) << "Chunk " << i 
							<< " has been found invalid, redownloading" << endl;
				
					resetChunk(i);
					tor.updateFilePercentage(i,bitset);
					saveIndexFile();
					recalc_chunks_left = true;
					corrupted_count++;
					corrupted(i);
					return 0;
				}
			}
			else
			{
				recheck_counter++;
			}
		}
		
		loaded.insert(i,bt::GetCurrentTime());
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
	
	void ChunkManager::resetChunk(unsigned int i)
	{
		if (i >= chunks.size())
			return;
		
		Chunk* c = chunks[i];
		if (c->getStatus() == Chunk::MMAPPED)
			cache->save(c);
		c->clear();
		c->setStatus(Chunk::NOT_DOWNLOADED);
		bitset.set(i,false);
		todo.set(i,!excluded_chunks.get(i) && !only_seed_chunks.get(i));
		loaded.remove(i);
		tor.updateFilePercentage(i,bitset);
	}
	
	void ChunkManager::checkMemoryUsage()
	{
		Uint32 num_removed = 0;
		QMap<Uint32,TimeStamp>::iterator i = loaded.begin();
		while (i != loaded.end())
		{
			Chunk* c = chunks[i.key()];
			// get rid of chunk if nobody asked for it in the last 5 seconds
			if (!c->taken() && bt::GetCurrentTime() - i.data() > 5000)
			{
				if (c->getStatus() == Chunk::MMAPPED)
					cache->save(c);
				c->clear();
				c->setStatus(Chunk::ON_DISK);
				QMap<Uint32,TimeStamp>::iterator j = i;
				i++;
				loaded.erase(j);
				num_removed++;
			}
			else
			{
				i++;
			}
		}
	//	Uint32 num_in_mem = loaded.count();
	//	Out() << QString("Cleaned %1 chunks, %2 still in memory").arg(num_removed).arg(num_in_mem) << endl;
	}
	
	void ChunkManager::saveChunk(unsigned int i,bool update_index)
	{
		if (i >= chunks.size())
			return;

		Chunk* c = chunks[i];
		if (!c->isExcluded())
		{
			cache->save(c);
			
			// update the index file
			if (update_index)
			{
				bitset.set(i,true);
				todo.set(i,false);
				recalc_chunks_left = true;
				writeIndexFileEntry(c);
				tor.updateFilePercentage(i,bitset);
			}
		}
		else
		{
			c->clear();
			c->setStatus(Chunk::NOT_DOWNLOADED);
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning: attempted to save a chunk which was excluded" << endl;
		}
	}

	void ChunkManager::writeIndexFileEntry(Chunk* c)
	{
		File fptr; 
		if (!fptr.open(index_file,"r+b"))
		{
			// no index file, so assume it's empty
			bt::Touch(index_file,true);
			Out(SYS_DIO|LOG_IMPORTANT) << "Can't open index file : " << fptr.errorString() << endl;
			// try again
			if (!fptr.open(index_file,"r+b"))
				// panick if it failes
				throw Error(i18n("Cannot open index file %1 : %2").arg(index_file).arg(fptr.errorString()));
		}

		
		fptr.seek(File::END,0);
		NewChunkHeader hdr;
		hdr.index = c->getIndex();
		fptr.write(&hdr,sizeof(NewChunkHeader));
	}
	
	Uint32 ChunkManager::onlySeedChunks() const
	{
		return only_seed_chunks.numOnBits();
	}
	
	bool ChunkManager::completed() const
	{
		return todo.numOnBits() == 0 && bitset.numOnBits() > 0;
	}
	
	Uint64 ChunkManager::bytesLeft() const
	{
		Uint32 num_left = bitset.getNumBits() - bitset.numOnBits();
		Uint32 last = chunks.size() - 1;
		if (last < chunks.size() && !bitset.get(last))
		{
			Chunk* c = chunks[last];
			if (c)
				return (num_left - 1)*tor.getChunkSize() + c->getSize();
			else
				return num_left*tor.getChunkSize();
		}
		else
		{
			return num_left*tor.getChunkSize();
		}
	}
	
	Uint64 ChunkManager::bytesLeftToDownload() const
	{
		Uint32 num_left = todo.numOnBits();
		Uint32 last = chunks.size() - 1;
		if (last < chunks.size() && todo.get(last))
		{
			Chunk* c = chunks[last];
			if (c)
				return (num_left - 1)*tor.getChunkSize() + c->getSize();
			else
				return num_left*tor.getChunkSize();
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
	
	bool ChunkManager::haveAllChunks() const
	{
		return bitset.numOnBits() == bitset.getNumBits();
	}

	Uint64 ChunkManager::bytesExcluded() const
	{
		Uint64 excl = 0;
		if (excluded_chunks.get(tor.getNumChunks() - 1))
		{
			Chunk* c = chunks[tor.getNumChunks() - 1];
			Uint32 num = excluded_chunks.numOnBits() - 1;
			excl = tor.getChunkSize() * num + c->getSize();
		}
		else
		{
			excl = tor.getChunkSize() * excluded_chunks.numOnBits();
		}
		
		if (only_seed_chunks.get(tor.getNumChunks() - 1))
		{
			Chunk* c = chunks[tor.getNumChunks() - 1];
			Uint32 num = only_seed_chunks.numOnBits() - 1;
			excl += tor.getChunkSize() * num + c->getSize();
		}
		else
		{
			excl += tor.getChunkSize() * only_seed_chunks.numOnBits();
		}
		return excl;
	}

	Uint32 ChunkManager::chunksExcluded() const
	{
		return excluded_chunks.numOnBits() + only_seed_chunks.numOnBits();
	}
	
	Uint32 ChunkManager::chunksDownloaded() const
	{
		return bitset.numOnBits();
	}
	
	void ChunkManager::debugPrintMemUsage()
	{
		Out(SYS_DIO|LOG_DEBUG) << "Active Chunks : " << loaded.count()<< endl;
	}

	void ChunkManager::prioritise(Uint32 from,Uint32 to,Priority priority)
	{
		if (from > to)
			std::swap(from,to);

		Uint32 i = from;
		while (i <= to && i < chunks.count())
		{
			Chunk* c = chunks[i];
			c->setPriority(priority);
			
			if (priority == ONLY_SEED_PRIORITY)
			{
				only_seed_chunks.set(i,true);
				todo.set(i,false);
			}
			else if (priority == EXCLUDED)
			{
				only_seed_chunks.set(i,false);
				todo.set(i,false);
			}
			else
			{
				only_seed_chunks.set(i,false);
				todo.set(i,!bitset.get(i));
			}
			
			i++;
		}
		updateStats();
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
			only_seed_chunks.set(i,false);
			todo.set(i,false);
			bitset.set(i,false);
			i++;
		}
		recalc_chunks_left = true;
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
			if (!bitset.get(i))
				todo.set(i,true);
			i++;
		}
		recalc_chunks_left = true;
		updateStats();
		included(from,to);
	}

	void ChunkManager::saveFileInfo()
	{
		// saves which TorrentFiles do not need to be downloaded
		File fptr;
		if (!fptr.open(file_info_file,"wb"))
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		// first write the number of excluded ones
		// don't know this yet, so write 0 for the time being
		Uint32 tmp = 0;
		fptr.write(&tmp,sizeof(Uint32));
		
		Uint32 i = 0;
		Uint32 cnt = 0;
		while (i < tor.getNumFiles())
		{
			if (tor.getFile(i).doNotDownload())
			{
				fptr.write(&i,sizeof(Uint32));
				cnt++;
			}
			i++;
		}

		// go back to the beginning and write the number of files
		fptr.seek(File::BEGIN,0);
		fptr.write(&cnt,sizeof(Uint32));
		fptr.flush();
	}
	
	void ChunkManager::loadFileInfo()
	{
		if (during_load)
			return;
		
		File fptr;
		if (!fptr.open(file_info_file,"rb"))
			return;

		Uint32 num = 0,tmp = 0;
		// first read the number of dnd files
		if (fptr.read(&num,sizeof(Uint32)) != sizeof(Uint32))
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : error reading chunk_info file" << endl;
			return;
		}

		for (Uint32 i = 0;i < num;i++)
		{
			if (fptr.read(&tmp,sizeof(Uint32)) != sizeof(Uint32))
			{
				Out(SYS_DIO|LOG_IMPORTANT) << "Warning : error reading chunk_info file" << endl;
				return;
			}

			bt::TorrentFile & tf = tor.getFile(tmp);
			if (!tf.isNull())
			{
				Out(SYS_DIO|LOG_DEBUG) << "Excluding : " << tf.getPath() << endl;
				tf.setDoNotDownload(true);
			}
		}
	}

	void ChunkManager::savePriorityInfo()
	{
		if (during_load)
			return;
		
		//save priority info and call saveFileInfo
		saveFileInfo();
		File fptr;
		if (!fptr.open(file_priority_file,"wb"))
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		try
		{
			// first write the number of excluded ones
			// don't know this yet, so write 0 for the time being
			Uint32 tmp = 0;
			fptr.write(&tmp,sizeof(Uint32));
		
			Uint32 i = 0;
			Uint32 cnt = 0;
			while (i < tor.getNumFiles())
			{
				const TorrentFile & tf = tor.getFile(i);
				if (tf.getPriority() != NORMAL_PRIORITY)
				{
					tmp = tf.getPriority();
					fptr.write(&i,sizeof(Uint32));
					fptr.write(&tmp,sizeof(Uint32));
					cnt+=2;
				}
				i++;
			}

			// go back to the beginning and write the number of items
			fptr.seek(File::BEGIN,0);
			fptr.write(&cnt,sizeof(Uint32));
			fptr.flush();
		}
		catch (bt::Error & err)
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Failed to save priority file " << err.toString() << endl;
			bt::Delete(file_priority_file,true);
		}
	}
	
	void ChunkManager::loadPriorityInfo()
	{ 
		//load priority info and if that fails load file info
		File fptr;
		if (!fptr.open(file_priority_file,"rb"))
		{
			loadFileInfo();
			return;
		}

		Uint32 num = 0;
		// first read the number of lines
		if (fptr.read(&num,sizeof(Uint32)) != sizeof(Uint32) || num > 2*tor.getNumFiles())
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : error reading chunk_info file" << endl;
			loadFileInfo();
			return;
		}

		Array<Uint32> buf(num);
		if (fptr.read(buf,sizeof(Uint32)*num) != sizeof(Uint32)*num)
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : error reading chunk_info file" << endl;
			loadFileInfo();
			return;
		}
		
		fptr.close();
		
		for (Uint32 i = 0;i < num;i += 2)
		{
			Uint32 idx = buf[i];
			if (idx >= tor.getNumFiles())
			{
				Out(SYS_DIO|LOG_IMPORTANT) << "Warning : error reading chunk_info file" << endl;
				loadFileInfo();
				return;
			}

			bt::TorrentFile & tf = tor.getFile(idx);
			
			if (!tf.isNull())
			{
				// numbers are to be compatible with old chunk info files
				switch(buf[i+1])
				{
				case FIRST_PRIORITY:
				case 3:
					tf.setPriority(FIRST_PRIORITY);
					break;
				case NORMAL_PRIORITY:
				case 2:
					tf.setPriority(NORMAL_PRIORITY);
					break;
				case EXCLUDED:
				case 0:
					//tf.setDoNotDownload(true);
					tf.setPriority(EXCLUDED);
					break;
				case ONLY_SEED_PRIORITY:
				case -1:
					tf.setPriority(ONLY_SEED_PRIORITY);
					break;
				default:
					tf.setPriority(LAST_PRIORITY);
					break;
				}
			}
		}
	}

	void ChunkManager::downloadStatusChanged(TorrentFile* tf,bool download)
	{
		Uint32 first = tf->getFirstChunk();
		Uint32 last = tf->getLastChunk();
		if (download)
		{
			// include the chunks 
			include(first,last);
			
			// if it is a multimedia file, prioritise first and last chunks of file
			if (tf->isMultimedia())
			{
				Uint32 chunkOffset;
				chunkOffset = ((last - first) / 100) + 1;

				prioritise(first,first+chunkOffset,PREVIEW_PRIORITY);
				if (last - first > 2)
				{
					prioritise(last - chunkOffset, last, PREVIEW_PRIORITY);
					//prioritise(last -1,last, PREVIEW_PRIORITY);
				}
			}
		}
		else
		{
		//	Out(SYS_DIO|LOG_DEBUG) << "Excluding chunks " << first << " to " << last << endl;
			// first and last chunk may be part of multiple files
			// so we can't just exclude them
			QValueList<Uint32> files,last_files;

			// get list of files where first chunk lies in
			tor.calcChunkPos(first,files);
			tor.calcChunkPos(last,last_files);
			// check for exceptional case which causes very long loops
			if (first == last && files.count() > 1)
			{
				cache->downloadStatusChanged(tf,download);
				savePriorityInfo();
				return;
			}
			
			// go over all chunks from first to last and mark them as not downloaded 
			// (first and last not included)
			for (Uint32 i = first + 1;i < last;i++)
				resetChunk(i);
			
			// if the first chunk only lies in one file, reset it
			if (files.count() == 1 && first != 0) 
			{
		//		Out(SYS_DIO|LOG_DEBUG) << "Resetting first " << first << endl;
				resetChunk(first);
			}
			
			// if the last chunk only lies in one file reset it
			if (last != first && last_files.count() == 1)
			{
		//		Out(SYS_DIO|LOG_DEBUG) << "Resetting last " << last << endl;
				resetChunk(last);
			}
			
			Priority maxp = ONLY_SEED_PRIORITY;
			bool reprioritise_border_chunk = false;
			bool modified = false;
			
			// if one file in the list needs to be downloaded,increment first
			for (QValueList<Uint32>::iterator i = files.begin();i != files.end();i++)
			{
				if (*i == tf->getIndex())
					continue;
				
				const TorrentFile & other = tor.getFile(*i);
				if (!other.doNotDownload())
				{
					if (first != last && !modified)
					{
						first++;
						reprioritise_border_chunk = true;
						modified = true;
					}
					
					if (other.getPriority() > maxp)
						maxp = other.getPriority();
				}
			}
			
			// in case we have incremented first, we better reprioritise the border chunk
			if (reprioritise_border_chunk)
				prioritise(first-1,first-1,maxp);
			
			maxp = ONLY_SEED_PRIORITY;
			reprioritise_border_chunk = false;
			modified = false;
			
			// if one file in the list needs to be downloaded,decrement last
			for (QValueList<Uint32>::iterator i = last_files.begin();i != last_files.end();i++)
			{
				if (*i == tf->getIndex())
					continue;
				
				const TorrentFile & other = tor.getFile(*i);
				if (!other.doNotDownload())
				{
					if (first != last && last > 0 && !modified)
					{
						last--;
						reprioritise_border_chunk = true;
						modified = true;
					}
					
					if (other.getPriority() > maxp)
						maxp = other.getPriority();
				}
			}
			
			if (reprioritise_border_chunk)
				prioritise(last+1,last+1,maxp);

			// last smaller then first is not normal, so just return
			if (last < first)
			{
				cache->downloadStatusChanged(tf,download);
				savePriorityInfo();
				return;
			}
			
		//	Out(SYS_DIO|LOG_DEBUG) << "exclude " << first << " to " << last << endl;
			exclude(first,last);
		}
		// alert the cache but first put things in critical operation mode
		cache->downloadStatusChanged(tf,download);
		savePriorityInfo();
	}

	void ChunkManager::downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority)
	{
		if (newpriority == EXCLUDED)
		{
			downloadStatusChanged(tf, false);
			return;
		}
		if (oldpriority == EXCLUDED)
		{
			downloadStatusChanged(tf, true);
			return;
		}

		savePriorityInfo();
		
		Uint32 first = tf->getFirstChunk();
		Uint32 last = tf->getLastChunk();
		
		// first and last chunk may be part of multiple files
		// so we can't just exclude them
		QValueList<Uint32> files;

		// get list of files where first chunk lies in
		tor.calcChunkPos(first,files);
		
		Chunk* c = chunks[first];
		// if one file in the list needs to be downloaded,increment first
		for (QValueList<Uint32>::iterator i = files.begin();i != files.end();i++)
		{
			Priority np = tor.getFile(*i).getPriority();
			if (np > newpriority && *i != tf->getIndex())
			{
				// make sure we don't go past last
				if (first == last)
					return;
					
				first++;
				break;
			}
		}
		
		files.clear();
		// get list of files where last chunk lies in
		tor.calcChunkPos(last,files);
		c = chunks[last];
		// if one file in the list needs to be downloaded,decrement last
		for (QValueList<Uint32>::iterator i = files.begin();i != files.end();i++)
		{
			Priority np = tor.getFile(*i).getPriority();
			if (np > newpriority && *i != tf->getIndex())
			{
				// make sure we don't wrap around
				if (last == 0 || last == first)
					return;
					
				last--;
				break;
			}
		}
		
		// last smaller then first is not normal, so just return
		if (last < first)
		{
			return;
		}
		

		prioritise(first,last,newpriority);
		if (newpriority == ONLY_SEED_PRIORITY)
			excluded(first,last);
	}
	
	bool ChunkManager::prepareChunk(Chunk* c,bool allways)
	{
		if (!allways && c->getStatus() != Chunk::NOT_DOWNLOADED)
			return false;
		
		return cache->prep(c);
	}
	
	QString ChunkManager::getOutputPath() const
	{
		return cache->getOutputPath();
	}
	
	void ChunkManager::preallocateDiskSpace(PreallocationThread* prealloc)
	{
		cache->preallocateDiskSpace(prealloc);
	}
	
	void ChunkManager::dataChecked(const BitSet & ok_chunks)
	{
		// go over all chunks at check each of them
		for (Uint32 i = 0;i < chunks.count();i++)
		{
			Chunk* c = chunks[i];
			if (ok_chunks.get(i) && !bitset.get(i))
			{
				// We think we do not hae a chunk, but we do have it
				bitset.set(i,true);
				todo.set(i,false);
				// the chunk must be on disk
				c->setStatus(Chunk::ON_DISK);
				tor.updateFilePercentage(i,bitset); 
			}
			else if (!ok_chunks.get(i) && bitset.get(i))
			{
				Out(SYS_DIO|LOG_IMPORTANT) << "Previously OK chunk " << i << " is corrupt !!!!!" << endl;
				// We think we have a chunk, but we don't
				bitset.set(i,false);
				todo.set(i,!only_seed_chunks.get(i) && !excluded_chunks.get(i));
				if (c->getStatus() == Chunk::ON_DISK)
				{
					c->setStatus(Chunk::NOT_DOWNLOADED);
					tor.updateFilePercentage(i,bitset);
				}
				else if (c->getStatus() == Chunk::MMAPPED || c->getStatus() == Chunk::BUFFERED)
				{
					resetChunk(i);
				}
				else
				{
					tor.updateFilePercentage(i,bitset);
				}
			}
		}
		recalc_chunks_left = true;
		try
		{
			saveIndexFile();
		}
		catch (bt::Error & err)
		{
			Out(SYS_DIO|LOG_DEBUG) << "Failed to save index file : " << err.toString() << endl;
		}
		catch (...)
		{
			Out(SYS_DIO|LOG_DEBUG) << "Failed to save index file : unkown exception" << endl;
		}
		chunksLeft();
		corrupted_count = 0;
	}
	
	bool ChunkManager::hasExistingFiles() const
	{
		return cache->hasExistingFiles();
	}
	
	
	void ChunkManager::recreateMissingFiles()
	{
		createFiles();
		if (tor.isMultiFile())
		{
			// loop over all files and mark all chunks of all missing files as
			// not downloaded
			for (Uint32 i = 0;i < tor.getNumFiles();i++)
			{
				TorrentFile & tf = tor.getFile(i);
				if (!tf.isMissing())
					continue;
				
				for (Uint32 j = tf.getFirstChunk(); j <= tf.getLastChunk();j++)
					resetChunk(j);
				tf.setMissing(false);
			}
		}
		else
		{
			// reset all chunks in case of single file torrent
			for (Uint32 j = 0; j < tor.getNumChunks();j++)
				resetChunk(j);
		}
		saveIndexFile();
		recalc_chunks_left = true;
		chunksLeft();
	}
	
	void ChunkManager::dndMissingFiles()
	{
	//	createFiles(); // create them again
		// loop over all files and mark all chunks of all missing files as
		// not downloaded
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (!tf.isMissing())
				continue;
				
			for (Uint32 j = tf.getFirstChunk(); j <= tf.getLastChunk();j++)
				resetChunk(j);
			tf.setMissing(false);
			tf.setDoNotDownload(true); // set do not download
		}
		savePriorityInfo();
		saveIndexFile();
		recalc_chunks_left = true;
		chunksLeft();
	}
	
	void ChunkManager::deleteDataFiles()
	{
		cache->deleteDataFiles();
	}

	Uint64 ChunkManager::diskUsage()
	{
		return cache->diskUsage();
	}

}

#include "chunkmanager.moc"
