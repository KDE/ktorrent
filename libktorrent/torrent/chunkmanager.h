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
#ifndef BTCHUNKMANAGER_H
#define BTCHUNKMANAGER_H

#include <qstring.h>
#include <qobject.h>
#include <qptrvector.h> 
#include "chunk.h"
#include <util/bitset.h>
#include "globals.h"

namespace bt
{
	class Torrent;
	class Cache;
	class TorrentFile;

	struct NewChunkHeader
	{
		unsigned int index; // the Chunks index
		unsigned int deprecated; // offset in cache file
	};
	
	/**
	 * @author Joris Guisson
	 * 
	 * Manages all Chunk's and the cache file, where all the chunk's are stored.
	 * It also manages a seperate index file, where the position of each piece
	 * in the cache file is stored.
	 *
	 * The chunks are stored in the cache file in the correct order. Eliminating
	 * the need for a file reconstruction algorithm for single files.
	 */
	class ChunkManager : public QObject
	{
		Q_OBJECT
				
		Torrent & tor;
		QString index_file,file_info_file;
		QPtrVector<Chunk> chunks;
		Uint32 num_chunks_in_cache_file;
		Uint32 max_allowed;
		Cache* cache;
		Uint32 num_in_mem;
		BitSet bitset,excluded_chunks;
		mutable Uint32 chunks_left;
		mutable bool recalc_chunks_left;
	public:
		ChunkManager(Torrent & tor,const QString & tmpdir,const QString & datadir);
		virtual ~ChunkManager();

		/**
		 * Change the data dir.
		 * @param data_dir 
		 */
		void changeDataDir(const QString & data_dir);
		
		/**
		 * Loads the index file.
		 * @throw Error When it can be loaded
		 */
		void loadIndexFile();
		
		/**
		 * Create the cache file, and index files.
		 * @throw Error When it can be created
		 */
		void createFiles();
		
		/**
		 * Get's the i'th Chunk.
		 * @param i The Chunk's index
		 * @return The Chunk, or 0 when i is out of bounds
		 */
		Chunk* getChunk(unsigned int i);
		
		/**
		 * Get's the i'th Chunk. Makes sure that the Chunk's data
		 * is in memory. If the Chunk hasn't been downloaded yet 0
		 * is returned. Whenever the Chunk needs to be uploaded, call
		 * this function. This changes the status to IN_MEMORY.
		 * @param i The Chunk's index
		 * @return The Chunk, or 0 when i is out of bounds
		 */
		Chunk* grabChunk(unsigned int i);
		
		/**
		 * The upload is done, and the Chunk is no longer needed.
		 * The Chunk's data might be cleared, if we are using up to much
		 * memory.
		 * @param i The Chunk's index
		 */
		void releaseChunk(unsigned int i);
		
		/**
		 * Save the i'th Chunk to the cache_file.
		 * Also changes the Chunk's status to ON_DISK.
		 * The Chunk's data is immediatly cleared.
		 * @param i The Chunk's index
		 */
		void saveChunk(unsigned int i);
		
		/**
		 * Calculates the number of bytes left to download.
		 * @return The number of bytes to download
		 */
		Uint64 bytesLeft() const;

		/**
		 * Calculates the number of bytes which have been excluded.
		 * @return The number of bytes excluded
		 */
		Uint64 bytesExcluded() const;
		
		/**
		 * Calculates the number of chunks left to download.
		 * @return The number of chunks to download
		 */
		Uint32 chunksLeft() const;

		/**
		 * Get the number of chunks which have been excluded.
		 * @return The number of excluded chunks
		 */
		Uint32 chunksExcluded() const;
		
		/**
		 * Get a BitSet of the status of all Chunks
		 */
		const BitSet & getBitSet() const {return bitset;}

		/**
		 * Get a BitSet of the status of all Chunks
		 */
		const BitSet & getExcludedBitSet() const {return excluded_chunks;}

		/// Get the number of chunks into the file.
		Uint32 getNumChunks() const {return chunks.count();}

		/**
		 * Get the highest chunk num, we are allowed to download.
		 * In order to avoid huge writes to the cache file in the beginning
		 * of the download. We artificially limit which pieces can be downloaded.
		 * 
		 * In the beggining we can only dowload the first 50 pieces. Once a piece
		 * comes in, we up the limit to that piece number + 50. Thus ensuring that
		 * the cache file will be expanded slowly.
		 * @return The maximum allowed chunk
		 */
		Uint32 getMaxAllowedChunk() const {return max_allowed;}

		/// Print memory usage to log file
		void debugPrintMemUsage();

		/**
		 * Check wether we're not using to much memory. And if necessary
		 * get rid of some chunks which aren't needed anymore.
		 */
		void checkMemoryUsage();

		/**
		 * Make sure that a range will get priority over other chunks.
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void prioritise(Uint32 from,Uint32 to);

		/**
		 * Make sure that a range will not be downloaded.
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void exclude(Uint32 from,Uint32 to);

		/**
		 * Make sure that a range will be downloaded.
		 * Does the opposite of exclude.
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void include(Uint32 from,Uint32 to);
	signals:
		/**
		 * Emitted when a range of chunks has been excluded
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void excluded(Uint32 from,Uint32 to);
		
		/**
		 * Emitted when chunks get excluded or included, so
		 * that the statistics can be updated.
		 */
		void updateStats();
		
	private:
		void saveIndexFile();
		void writeIndexFileEntry(Chunk* c);
		void saveFileInfo();
		void loadFileInfo();

	private slots:
		void downloadStatusChanged(TorrentFile* tf,bool download);
	};

}

#endif
