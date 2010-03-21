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

#include <QMap>
#include <QSet>
#include <QString>
#include <QObject>
#include <vector> 
#include <util/bitset.h>
#include <torrent/torrent.h>
#include <btcore_export.h>
#include "chunk.h"


class QStringList;

namespace bt
{
	class Cache;
	class TorrentFile;
	class PreallocationThread;
	class TorrentFileInterface;
	class CacheFactory;
	class Job;

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
	class BTCORE_EXPORT ChunkManager : public QObject
	{
		Q_OBJECT
				
		Torrent & tor;
		QString index_file,file_info_file,file_priority_file;
		std::vector<Chunk*> chunks;
		Cache* cache;
		BitSet bitset;
		BitSet excluded_chunks;
		BitSet only_seed_chunks;
		BitSet todo;
		mutable Uint32 chunks_left;
		mutable bool recalc_chunks_left;
		Uint32 corrupted_count;
		Uint32 recheck_counter;
		bool during_load;	
		QSet<Uint32> border_chunks;
	public:
		ChunkManager(Torrent & tor,
					 const QString & tmpdir,
					 const QString & datadir,
					 bool custom_output_name,
					 CacheFactory* fac);
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
		 */
		Job* moveDataFiles(const QString & ndir);
		
		/**
		 * A move of data files has finished
		 * @param job The job doing the move
		 */
		void moveDataFilesFinished(Job* job);
		
		/**
		 * Move some data files to a new location
		 * @param files Map of files and their new location
		 */
		Job* moveDataFiles(const QMap<TorrentFileInterface*,QString> & files);
		
		
		/**
		 * A move of data files with the map has finished
		 * @param files Map of files and their new location
		 * @param job The job doing the move
		 */
		void moveDataFilesFinished(const QMap<TorrentFileInterface*,QString> & files,Job* job);
		
		/**
		 * Loads the index file.
		 * @throw Error When it can be loaded
		 */
		void loadIndexFile();
		
		/**
		 * Create the cache file, and index files.
		 * @param check_priority Check if priority of chunk matches that of files
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
		 * Reset a chunk as if it were never downloaded.
		 * @param i The chunk
		 */
		void resetChunk(unsigned int i);
		
		/**
		 * Mark a chunk as downloaded.
		 * @param i The Chunk's index
		 */
		void chunkDownloaded(unsigned int i);
		
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
		Uint32 getNumChunks() const {return chunks.size();}

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
		
		/// Mark all existing files as downloaded
		void markExistingFilesAsDownloaded();
		
		/// Recreates missing files
		void recreateMissingFiles();
		
		/// Set missing files as do not download
		void dndMissingFiles();
		
		/// Delete all data files 
		Job* deleteDataFiles();
		
		/// Are all not deselected chunks downloaded.
		bool completed() const;
		
		/// Set the maximum chunk size for a data check, 0 means there is no limit
		static void setMaxChunkSizeForDataCheck(Uint32 mcs) {max_chunk_size_for_data_check = mcs;}

		/// Enabled or disable data checking during upload
		static void setUploadDataCheckingEnabled(bool on) {do_data_check = on;}
		
		/// Set the preview sizes for audio and video files
		static void setPreviewSizes(Uint32 audio,Uint32 video);

		/// Get the current disk usage of all the files in this torrent
		Uint64 diskUsage();
		
		/// Get the size in chunks of the preview range of a file of the torrent
		Uint32 previewChunkRangeSize(const TorrentFile & tf) const;
		
		/// Get the size in chunks of the preview range for a single file torrent
		Uint32 previewChunkRangeSize() const;
		
		/// The download priority of a file has changed
		void downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority);
		
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
		void doPreviewPriority(TorrentFile & tf);
		bool allFilesExistOfChunk(Uint32 idx);
		bool isBorderChunk(Uint32 idx) const;
		void setBorderChunkPriority(Uint32 idx,Priority prio);
		bool resetBorderChunk(Uint32 idx,TorrentFile* tf);
		void createBorderChunkSet();
		void dumpPriority(TorrentFile* tf);
		void downloadStatusChanged(TorrentFile* tf,bool download);
		
	private:
		static bool do_data_check;
		static Uint32 max_chunk_size_for_data_check;
		static Uint32 preview_size_audio;
		static Uint32 preview_size_video;
	};

}

#endif
