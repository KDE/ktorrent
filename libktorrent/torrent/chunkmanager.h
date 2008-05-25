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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTCHUNKMANAGER_H
#define BTCHUNKMANAGER_H

#include <qmap.h>
#include <qstring.h>
#include <qobject.h>
#include <qptrvector.h> 
#include <util/bitset.h>
#include "chunk.h"
#include "globals.h"

class QStringList;

namespace KIO
{
	class Job;
}

namespace bt
{
	class Torrent;
	class Cache;
	class TorrentFile;
	class PreallocationThread;

	struct NewChunkHeader
	{
		unsigned int index; // the Chunks index
		unsigned int deprecated; // offset in cache file
	};
	
	/**
	 * @author Joris Guisson
	 * 
	 * Manages all Chunk's and the cache file, where all the chunk's are stored.
	 * It also manages a separate index file, where the position of each piece
	 * in the cache file is stored.
	 *
	 * The chunks are stored in the cache file in the correct order. Eliminating
	 * the need for a file reconstruction algorithm for single files.
	 */
	class ChunkManager : public QObject
	{
		Q_OBJECT
				
		Torrent & tor;
		QString index_file,file_info_file,file_priority_file;
		QPtrVector<Chunk> chunks;
		Cache* cache;
		QMap<Uint32,TimeStamp> loaded; // loaded chunks and when they were loaded
		BitSet bitset;
		BitSet excluded_chunks;
		BitSet only_seed_chunks;
		BitSet todo;
		mutable Uint32 chunks_left;
		mutable bool recalc_chunks_left;
		Uint32 corrupted_count;
		Uint32 recheck_counter;
		bool during_load;	
	public:
		ChunkManager(Torrent & tor,
					 const QString & tmpdir,
					 const QString & datadir,
					 bool custom_output_name);
		virtual ~ChunkManager();

		/// Get the torrent
		const Torrent & getTorrent() const {return tor;}
		
		/// Get the data dir
		QString getDataDir() const;
		
		/// Get the actual output path
		QString getOutputPath() const;
		
		void changeOutputPath(const QString& output_path);
		
		/// Remove obsolete chunks
		void checkMemoryUsage();
		
		/**
		 * Change the data dir.
		 * @param data_dir 
		 */
		void changeDataDir(const QString & data_dir);
		
		/**
		 * Move the data files of the torrent.
		 * @param ndir The new directory
		 * @return The job doing the move
		 */
		KIO::Job* moveDataFiles(const QString & ndir);
		
		/**
		 * The move data files job has finished
		 * @param job The move job
		 */
		void moveDataFilesCompleted(KIO::Job* job);
		
		/**
		 * Loads the index file.
		 * @throw Error When it can be loaded
		 */
		void loadIndexFile();
		
		/**
		 * Create the cache file, and index files.
		 * @param check_priority Make sure chunk priorities and dnd status of files match
		 * @throw Error When it can be created
		 */
		void createFiles(bool check_priority = false);
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		bool hasMissingFiles(QStringList & sl);
		
		/**
		 * Preallocate diskspace for all files
		 * @param prealloc The thread doing the preallocation
		 */
		void preallocateDiskSpace(PreallocationThread* prealloc);
		
		/**
		 * Open the necessary files when the download gets started.
		 */
		void start();
		
		/**
		 * Closes files when the download gets stopped.
		 */
		void stop();
		
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
		 * this function. This changes the status to MMAPPED or BUFFERED.
		 * @param i The Chunk's index
		 * @return The Chunk, or 0 when i is out of bounds
		 */
		Chunk* grabChunk(unsigned int i);
		
		/**
		 * Prepare a chunk for downloading
		 * @param c The Chunk
		 * @param allways Always do this, even if the chunk is not NOT_DOWNLOADED
		 * @return true if ok, false if the chunk is not NOT_DOWNLOADED
		 */
		bool prepareChunk(Chunk* c,bool allways = false);
		
		/**
		 * The upload is done, and the Chunk is no longer needed.
		 * The Chunk's data might be cleared, if we are using up to much
		 * memory.
		 * @param i The Chunk's index
		 */
		void releaseChunk(unsigned int i);
		
		/**
		 * Reset a chunk as if it were never downloaded.
		 * @param i The chunk
		 */
		void resetChunk(unsigned int i);
		
		/**
		 * Save the i'th Chunk to the cache_file.
		 * Also changes the Chunk's status to ON_DISK.
		 * The Chunk's data is immediately cleared.
		 * @param i The Chunk's index
		 * @param update_index Update the index or not
		 */
		void saveChunk(unsigned int i,bool update_index = true);
		
		/**
		 * Calculates the number of bytes left for the tracker. Does include 
		 * excluded chunks (this should be used for the tracker).
		 * @return The number of bytes to download + the number of bytes excluded
		 */
		Uint64 bytesLeft() const;
		
		/**
		 * Calculates the number of bytes left to download.
		 */
		Uint64 bytesLeftToDownload() const;

		/**
		 * Calculates the number of bytes which have been excluded.
		 * @return The number of bytes excluded
		 */
		Uint64 bytesExcluded() const;
		
		/**
		 * Calculates the number of chunks left to download. 
		 * Does not include excluded chunks.
		 * @return The number of chunks to download
		 */
		Uint32 chunksLeft() const;
		
		/**
		 * Check if we have all chunks, this is not the same as
		 * chunksLeft() == 0, it does not look at excluded chunks.
		 * @return true if all chunks have been downloaded
		 */
		bool haveAllChunks() const;

		/**
		 * Get the number of chunks which have been excluded.
		 * @return The number of excluded chunks
		 */
		Uint32 chunksExcluded() const;
		
		/**
		 * Get the number of downloaded chunks
		 * @return 
		 */
		Uint32 chunksDownloaded() const;
		
		/**
		 * Get the number of only seed chunks.
		 */
		Uint32 onlySeedChunks() const;
		
		/**
		 * Get a BitSet of the status of all Chunks
		 */
		const BitSet & getBitSet() const {return bitset;}

		/**
		 * Get the excluded bitset
		 */
		const BitSet & getExcludedBitSet() const {return excluded_chunks;}
		
		/**
		 * Get the only seed bitset. 
		 */
		const BitSet & getOnlySeedBitSet() const {return only_seed_chunks;}

		/// Get the number of chunks into the file.
		Uint32 getNumChunks() const {return chunks.count();}

		/// Print memory usage to log file
		void debugPrintMemUsage();

		/**
		 * Make sure that a range will get priority over other chunks.
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void prioritise(Uint32 from,Uint32 to, Priority priority);

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
		
				
		/**
		 * Data has been checked, and these chunks are OK.
		 * The ChunkManager will update it's internal structures
		 * @param ok_chunks The ok_chunks
		 */
		void dataChecked(const BitSet & ok_chunks);
		
		/// Test if the torrent has existing files, only works the first time a torrent is loaded
		bool hasExistingFiles() const;
		
		/// Recreates missing files
		void recreateMissingFiles();
		
		/// Set missing files as do not download
		void dndMissingFiles();
		
		/// Delete all data files 
		void deleteDataFiles();
		
		/// Are all not deselected chunks downloaded.
		bool completed() const;
		
		/// Set the maximum chunk size for a data check, 0 means alllways check
		static void setMaxChunkSizeForDataCheck(Uint32 mcs) {max_chunk_size_for_data_check = mcs;}

		/// Get the current disk usage of all the files in this torrent
		Uint64 diskUsage();
	signals:
		/**
		 * Emitted when a range of chunks has been excluded
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void excluded(Uint32 from,Uint32 to);
		
		/**
		 * Emitted when a range of chunks has been included back.
		 * @param from First chunk in range
		 * @param to Last chunk in range
		 */
		void included(Uint32 from,Uint32 to);
		
		/**
		 * Emitted when chunks get excluded or included, so
		 * that the statistics can be updated.
		 */
		void updateStats();
		
		/**
		 * A corrupted chunk has been found during uploading.
		 * @param chunk The chunk
		 */
		void corrupted(Uint32 chunk);
		
	private:
		void saveIndexFile();
		void writeIndexFileEntry(Chunk* c);
		void saveFileInfo();
		void loadFileInfo();
		void savePriorityInfo();
		void loadPriorityInfo();

	private slots:
		void downloadStatusChanged(TorrentFile* tf,bool download);
		void downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority);
		
		static Uint32 max_chunk_size_for_data_check;
	};

}

#endif
