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
#include "chunkmanager.h"
#include <algorithm>
#include <kmimetype.h>
#include <util/file.h>
#include <util/array.h>
#include <qstringlist.h>
#include <torrent/torrent.h>
#include <util/error.h>
#include <util/bitset.h>
#include <util/fileops.h>
#include "singlefilecache.h"
#include "multifilecache.h"
#include <util/log.h>
#include <util/functions.h>
#include <interfaces/cachefactory.h>
#include <util/timer.h>
#include <klocale.h>

namespace bt
{
	
	Uint32 ChunkManager::max_chunk_size_for_data_check = 512 * 1024;
	bool ChunkManager::do_data_check = true;
	Uint32 ChunkManager::preview_size_audio = 256 * 1024; // 256 KB for audio files
	Uint32 ChunkManager::preview_size_video = 2048 * 1024; // 2 MB for videos
	

	ChunkManager::ChunkManager(
			Torrent & tor,
			const QString & tmpdir,
			const QString & datadir,
			bool custom_output_name,
			CacheFactory* fac)
	: tor(tor),chunks(tor.getNumChunks()),
	bitset(tor.getNumChunks()),excluded_chunks(tor.getNumChunks()),only_seed_chunks(tor.getNumChunks()),todo(tor.getNumChunks())
	{
		during_load = false;
		only_seed_chunks.setAll(false);
		todo.setAll(true);
		
		if (!fac)
		{
			if (tor.isMultiFile())
				cache = new MultiFileCache(tor,tmpdir,datadir,custom_output_name);
			else
				cache = new SingleFileCache(tor,tmpdir,datadir);
		}
		else
			cache = fac->create(tor,tmpdir,datadir);
		
		cache->loadFileMap();
		index_file = tmpdir + "index";
		file_info_file = tmpdir + "file_info";
		file_priority_file = tmpdir + "file_priority";
		Uint64 tsize = tor.getTotalSize();	// total size
		Uint64 csize = tor.getChunkSize();	// chunk size
		Uint64 lsize = tsize - (csize * (tor.getNumChunks() - 1)); // size of last chunk
		
		for (Uint32 i = 0;i < tor.getNumChunks();i++)
		{
			if (i + 1 < tor.getNumChunks())
				chunks[i] = new Chunk(i,csize,cache);
			else
				chunks[i] = new Chunk(i,lsize,cache);
		}
		
		chunks_left = 0;
		recalc_chunks_left = true;
		corrupted_count = recheck_counter = 0;

		if (tor.isMultiFile())
			createBorderChunkSet();
		
		if (tor.isMultiFile())
		{
			Uint32 nfiles = tor.getNumFiles();
			for (Uint32 i = 0;i < nfiles;i++)
			{
				TorrentFile & tf = tor.getFile(i);
				if (tf.isMultimedia())
					doPreviewPriority(tf);
			}
		}
		else if (tor.isMultimedia())
		{
			Uint32 nchunks = previewChunkRangeSize();

			prioritise(0,nchunks,PREVIEW_PRIORITY);
			if (tor.getNumChunks() > nchunks)
			{
				prioritise(tor.getNumChunks() - nchunks, tor.getNumChunks() - 1,PREVIEW_PRIORITY);
			}
		}
	}


	ChunkManager::~ChunkManager()
	{
		for (Uint32 i = 0;i < (Uint32)chunks.size();i++)
		{
			Chunk* c = chunks[i];
			delete c;
		}

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
	
	void ChunkManager::changeOutputPath(const QString & output_path)
	{
		cache->changeOutputPath(output_path);
	}
	
	Job* ChunkManager::moveDataFiles(const QString & ndir)
	{
		return cache->moveDataFiles(ndir);
	}
	
	void ChunkManager::moveDataFilesFinished(Job* job)
	{
		cache->moveDataFilesFinished(job);
	}
	
	Job* ChunkManager::moveDataFiles(const QMap<TorrentFileInterface*,QString> & files)
	{
		return cache->moveDataFiles(files);
	}
	
	void ChunkManager::moveDataFilesFinished(const QMap<TorrentFileInterface*,QString> & files,Job* job)
	{
		cache->moveDataFilesFinished(files,job);
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
			Out(SYS_DIO|LOG_IMPORTANT) << "Can not open index file : " << fptr.errorString() << endl;
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
		tor.updateFilePercentage(*this);
		during_load = false;
	}
	
	void ChunkManager::saveIndexFile()
	{
		File fptr;
		if (!fptr.open(index_file,"wb"))
			throw Error(i18n("Cannot open index file %1 : %2",index_file,fptr.errorString()));
		
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
			during_load = true; // for performance reasons
			for (Uint32 i = 0;i < tor.getNumFiles();i++)
			{
				TorrentFile & tf = tor.getFile(i);				
				if (tf.getPriority() != NORMAL_PRIORITY)
				{
					downloadPriorityChanged(&tf,tf.getPriority(),tf.getOldPriority());
				}
			}
			during_load = false;
			savePriorityInfo();
		}
	}
	
	bool ChunkManager::hasMissingFiles(QStringList & sl)
	{
		return cache->hasMissingFiles(sl);
	}

	Chunk* ChunkManager::getChunk(unsigned int i)
	{
		if (i >= (Uint32)chunks.size())
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
		cache->close();
	}
	
	void ChunkManager::resetChunk(unsigned int i)
	{
		if (i >= (Uint32)chunks.size() || during_load)
			return;
		
		Chunk* c = chunks[i];
		cache->clearPieces(c);
		c->setStatus(Chunk::NOT_DOWNLOADED);
		bitset.set(i,false);
		todo.set(i,!excluded_chunks.get(i) && !only_seed_chunks.get(i));
		tor.updateFilePercentage(i,*this);
		Out(SYS_DIO|LOG_DEBUG) << QString("Resetted chunk %1").arg(i) << endl;
	}
	
	void ChunkManager::checkMemoryUsage()
	{
		cache->checkMemoryUsage();
	}
	
	void ChunkManager::chunkDownloaded(unsigned int i)
	{
		if (i >= (Uint32)chunks.size())
			return;

		Chunk* c = chunks[i];
		if (!c->isExcluded())
		{
			// update the index file
			bitset.set(i,true);
			todo.set(i,false);
			recalc_chunks_left = true;
			writeIndexFileEntry(c);
			c->setStatus(Chunk::ON_DISK);
			tor.updateFilePercentage(i,*this);
		}
		else
		{
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
			Out(SYS_DIO|LOG_IMPORTANT) << "Can not open index file : " << fptr.errorString() << endl;
			// try again
			if (!fptr.open(index_file,"r+b"))
				// panick if it failes
				throw Error(i18n("Cannot open index file %1 : %2",index_file,fptr.errorString()));
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
		if (last < (Uint32)chunks.size() && !bitset.get(last))
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
		if (last < (Uint32)chunks.size() && todo.get(last))
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
			if (c && !bitset.get(i) && !c->isExcluded())
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
	//	Out(SYS_DIO|LOG_DEBUG) << "Active Chunks : " << loaded.count()<< endl;
	}

	void ChunkManager::prioritise(Uint32 from,Uint32 to,Priority priority)
	{
		if (from > to)
			std::swap(from,to);

		Uint32 i = from;
		while (i <= to && i < (Uint32)chunks.size())
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
		while (i <= to && i < (Uint32)chunks.size())
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
		while (i <= to && i < (Uint32)chunks.size())
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
		if (during_load)
			return;
		
		// saves which TorrentFiles do not need to be downloaded
		File fptr;
		if (!fptr.open(file_info_file,"wb"))
		{
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : Can not save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		QList<Uint32> dnd;
		
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
		for (i = 0;i < (Uint32)dnd.count();i++)
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
			Out(SYS_DIO|LOG_IMPORTANT) << "Warning : Can not save chunk_info file : " << fptr.errorString() << endl;
			return;
		}

		try
		{
			QList<Uint32> dnd;
			
			Uint32 i = 0;
			for ( ; i < tor.getNumFiles(); i++)
			{
				if(tor.getFile(i).getPriority() != NORMAL_PRIORITY)
				{
					dnd.append(i);
					dnd.append(tor.getFile(i).getPriority());
				}
			}
	
			Uint32 tmp = dnd.count();
			fptr.write(&tmp,sizeof(Uint32));
			// write all the non-default priority ones
			for (i = 0;i < (Uint32)dnd.count();i++)
			{
				tmp = dnd[i];
				fptr.write(&tmp,sizeof(Uint32));
			}
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
					// By default priority is set to normal, so do nothing
					//tf.setPriority(NORMAL_PRIORITY);
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
				doPreviewPriority(*tf);
			}
		}
		else
		{
			// check for exceptional case which causes very long loops
			// check for exceptional case which causes very long loops
			if (first == last)
			{
				if (!isBorderChunk(first))
				{
					resetChunk(first);
					exclude(first,first);
				}
				else
				{
					if (resetBorderChunk(last,tf)) // try resetting it
						exclude(first,last);
				}
				cache->downloadStatusChanged(tf,download);
				savePriorityInfo();
				if (!during_load)
					tor.updateFilePercentage(*this);
				return;
			}
			
			// go over all chunks from first to last and mark them as not downloaded 
			// (first and last not included)
			for (Uint32 i = first + 1;i < last;i++)
				resetChunk(i);
			
			// if the first chunk only lies in one file, reset it
			if (!isBorderChunk(first)) 
				resetChunk(first);
			else if (!resetBorderChunk(first,tf))
				// try to reset if it lies in multiple files
				first++;
			
			if (last != first)
			{
				// if the last chunk only lies in one file reset it
				if (!isBorderChunk(last))
					resetChunk(last);
				else if (!resetBorderChunk(last,tf))
					last--;
			}
		
			if (first <= last)
				exclude(first,last);
		}
	
		cache->downloadStatusChanged(tf,download);
		savePriorityInfo();
		if (!during_load)
			tor.updateFilePercentage(*this);
	}

	void ChunkManager::downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority)
	{
		if (newpriority == EXCLUDED)
		{
			downloadStatusChanged(tf, false);
		//	dumpPriority(tf);
			return;
		}
		
		if (oldpriority == EXCLUDED)
		{
			downloadStatusChanged(tf, true);
		}

		savePriorityInfo();
		
		Uint32 first = tf->getFirstChunk();
		Uint32 last = tf->getLastChunk();
		
		if (first == last)
		{
			if (isBorderChunk(first))
				setBorderChunkPriority(first,newpriority);
			else
				prioritise(first,first,newpriority);
			
			if (newpriority == ONLY_SEED_PRIORITY)
				excluded(first,last);
		}
		else
		{
			// if the first one is a border chunk use setBorderChunkPriority and make the range smaller
			if (isBorderChunk(first))
			{
				setBorderChunkPriority(first,newpriority);
				first++;
			}
			
			// if the last one is a border chunk use setBorderChunkPriority and make the range smaller
			if (isBorderChunk(last))
			{
				setBorderChunkPriority(last,newpriority);
				last--;
			}
			
			// if we still have a valid range, prioritise it
			if (first <= last)
			{
				prioritise(first,last,newpriority);
				if (newpriority == ONLY_SEED_PRIORITY)
					excluded(first,last);
			}
		}
		
		// if it is a multimedia file, make sure we haven't overridden preview priority
		if (tf->isMultimedia())
		{
			doPreviewPriority(*tf);
		}
		//dumpPriority(tf);
	}
	
	bool ChunkManager::isBorderChunk(Uint32 idx) const
	{
		return border_chunks.contains(idx);
	}
	
	void ChunkManager::setBorderChunkPriority(Uint32 idx,Priority prio)
	{
		QList<Uint32> files;

		Priority highest = prio;
		// get list of files where first chunk lies in
		tor.calcChunkPos(idx,files);
		foreach (Uint32 file,files)
		{
			Priority np = tor.getFile(file).getPriority();
			if (np > highest)
				highest = np;
		}
		prioritise(idx,idx,highest);
		if (highest == ONLY_SEED_PRIORITY)
			excluded(idx,idx);
	}
	
	bool ChunkManager::resetBorderChunk(Uint32 idx,TorrentFile* tf)
	{
		QList<Uint32> files;
		tor.calcChunkPos(idx,files);
		foreach (Uint32 file,files)
		{
			const TorrentFile & other = tor.getFile(file);
			if (file == tf->getIndex())
				continue;
			
			// This file needs to be downloaded, so we can't reset the chunk
			if (!other.doNotDownload())
			{
				// Priority might need to be modified, so set it's priority 
				// to the maximum of all the files who still need it
				setBorderChunkPriority(idx,other.getPriority());
				return false;
			}
		}
		
		// we can reset safely
		resetChunk(idx);
		return true;
	}
	
	void ChunkManager::createBorderChunkSet()
	{
		// figure out border chunks
		for (Uint32 i = 0;i < tor.getNumFiles() - 1;i++)
		{
			TorrentFile & a = tor.getFile(i);
			TorrentFile & b = tor.getFile(i+1);
			if (a.getLastChunk() == b.getFirstChunk())
				border_chunks.insert(a.getLastChunk());
		}
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
		for (Uint32 i = 0;i < (Uint32)chunks.size();i++)
		{
			Chunk* c = chunks[i];
			if (ok_chunks.get(i) && !bitset.get(i))
			{
				// We think we do not have a chunk, but we do have it
				bitset.set(i,true);
				todo.set(i,false);
				// the chunk must be on disk
				c->setStatus(Chunk::ON_DISK);
				tor.updateFilePercentage(i,*this); 
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
					tor.updateFilePercentage(i,*this);
				}
				else
				{
					tor.updateFilePercentage(i,*this);
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
			Out(SYS_DIO|LOG_DEBUG) << "Failed to save index file : unknown exception" << endl;
		}
		chunksLeft();
		corrupted_count = 0;
	}
	
	bool ChunkManager::hasExistingFiles() const
	{
		return cache->hasExistingFiles();
	}
	
	bool ChunkManager::allFilesExistOfChunk(Uint32 idx)
	{
		QList<Uint32> files;
		tor.calcChunkPos(idx,files);
		foreach (Uint32 fidx,files)
		{
			TorrentFile & tf = tor.getFile(fidx);
			if (!tf.isPreExistingFile())
				return false;
		}
		return true;
	}
	
	void ChunkManager::markExistingFilesAsDownloaded()
	{
		if (tor.isMultiFile())
		{
			// loop over all files and mark all chunks of all existing files as
			// downloaded
			for (Uint32 i = 0;i < tor.getNumFiles();i++)
			{
				TorrentFile & tf = tor.getFile(i);
				if (!tf.isPreExistingFile())
					continue;
				
				// all the chunks in the middle of the file are OK
				for (Uint32 j = tf.getFirstChunk() + 1;j < tf.getLastChunk();j++)
				{
					Chunk* c = chunks[j];
					c->setStatus(Chunk::ON_DISK);
					bitset.set(j,true);
					todo.set(j,false);
					tor.updateFilePercentage(j,*this); 
				}
				
				// all files of the first chunk must be preexisting
				if (allFilesExistOfChunk(tf.getFirstChunk()))
				{
					Uint32 idx = tf.getFirstChunk();
					Chunk* c = chunks[idx];
					c->setStatus(Chunk::ON_DISK);
					bitset.set(idx,true);
					todo.set(idx,false);
					tor.updateFilePercentage(idx,*this); 
				}
				
				// all files of the last chunk must be preexisting
				if (allFilesExistOfChunk(tf.getLastChunk()))
				{
					Uint32 idx = tf.getLastChunk();
					Chunk* c = chunks[idx];
					c->setStatus(Chunk::ON_DISK);
					bitset.set(idx,true);
					todo.set(idx,false);
					tor.updateFilePercentage(idx,*this); 
				}
			}
		}
		else if (cache->hasExistingFiles())
		{
			for (Uint32 i = 0;i < chunks.size();i++)
			{
				Chunk* c = chunks[i];
				c->setStatus(Chunk::ON_DISK);
				bitset.set(i,true);
				todo.set(i,false);
				tor.updateFilePercentage(i,*this); 
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
			Out(SYS_DIO|LOG_DEBUG) << "Failed to save index file : unknown exception" << endl;
		}
		chunksLeft();
		corrupted_count = 0;
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
	
	Job* ChunkManager::deleteDataFiles()
	{
		return cache->deleteDataFiles();
	}

	Uint64 ChunkManager::diskUsage()
	{
		return cache->diskUsage();
	}
	
	Uint32 ChunkManager::previewChunkRangeSize(const TorrentFile & file) const
	{
		if (!file.isMultimedia())
			return 0;
		
		if (file.getFirstChunk() == file.getLastChunk())
			return 1;
		
		Uint32 preview_size = 0;
		if (file.isVideo())
			preview_size = preview_size_video;
		else
			preview_size = preview_size_audio;
			
		Uint32 nchunks = preview_size / tor.getChunkSize();
		if (nchunks == 0)
			nchunks = 1;
		
		return nchunks;
	}
		
	
	Uint32 ChunkManager::previewChunkRangeSize() const
	{
		KMimeType::Ptr ptr = KMimeType::findByPath(tor.getNameSuggestion());
		Uint32 preview_size = 0;
		if (ptr->name().startsWith("video"))
			preview_size = preview_size_video;
		else
			preview_size = preview_size_audio;
				
		Uint32 nchunks = preview_size / tor.getChunkSize();
		if (nchunks == 0)
			nchunks = 1;
		return nchunks;
	}
	
	void ChunkManager::doPreviewPriority(TorrentFile & file)
	{
		if (file.getPriority() == EXCLUDED || file.getPriority() == ONLY_SEED_PRIORITY)
			return;
		
		if (file.getFirstChunk() == file.getLastChunk())
		{
			// prioritise whole file 
			prioritise(file.getFirstChunk(),file.getLastChunk(),PREVIEW_PRIORITY);
			return;
		}
		
		Uint32 nchunks = previewChunkRangeSize(file);
		if (!nchunks)
			return;
		
		prioritise(file.getFirstChunk(), file.getFirstChunk()+nchunks, PREVIEW_PRIORITY);
		if (file.getLastChunk() - file.getFirstChunk() > nchunks)
		{
			prioritise(file.getLastChunk() - nchunks, file.getLastChunk(), PREVIEW_PRIORITY);
		}	
	}
	
	void ChunkManager::setPreviewSizes(Uint32 audio,Uint32 video)
	{
		preview_size_audio = audio;
		preview_size_video = video;
	}
	
	void ChunkManager::dumpPriority(TorrentFile* tf)
	{
		Uint32 first = tf->getFirstChunk();
		Uint32 last = tf->getLastChunk();
		Out(SYS_DIO|LOG_DEBUG) << "DumpPriority : " << tf->getPath() << " " << first << " " << last << endl;
		for (Uint32 i = first;i <= last;i++)
		{
			QString prio;
			switch (chunks[i]->getPriority())
			{
				case FIRST_PRIORITY: prio = "First"; break;
				case LAST_PRIORITY:	 prio = "Last"; break;
				case ONLY_SEED_PRIORITY:  prio = "Only Seed"; break;
				case EXCLUDED:  prio = "Excluded"; break;
				case PREVIEW_PRIORITY:  prio = "Preview"; break;
				default:  prio = "Normal"; break;
					
			}
			Out(SYS_DIO|LOG_DEBUG) << i << " prio " << prio << endl;
		}
	}
}

#include "chunkmanager.moc"
