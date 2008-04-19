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
#ifndef BTCACHE_H
#define BTCACHE_H

#include <kio/job.h>

class QStringList;

namespace bt
{
	class Torrent;
	class TorrentFile;
	class Chunk;
	class PreallocationThread;


	/**
	 * @author Joris Guisson
	 * @brief Manages the temporary data
	 *
	 * Interface for a class which manages downloaded data.
	 * Subclasses should implement the load and save methods.
	 */
	class Cache
	{
	protected:
		Torrent & tor;
		QString tmpdir;
		QString datadir;
		bool preexisting_files;
		Uint32 mmap_failures;
	public:
		Cache(Torrent & tor,const QString & tmpdir,const QString & datadir);
		virtual ~Cache();

		/// Get the datadir
		QString getDataDir() const {return datadir;}
		
		/**
		 * Get the actual output path.
		 * @return The output path
		 */
		virtual QString getOutputPath() const = 0;
		
		/**
		 * Changes the tmp dir. All data files should already been moved.
		 * This just modifies the tmpdir variable.
		 * @param ndir The new tmpdir
		 */
		virtual void changeTmpDir(const QString & ndir);
		
		/**
		 * Move the data files to a new directory.
		 * @param ndir The directory
		 * @return The KIO::Job doing the move
		 */
		virtual KIO::Job* moveDataFiles(const QString & ndir) = 0;
		
		/**
		 * The move data files job is done.
		 * @param job The job that did it 
		 */
		virtual void moveDataFilesCompleted(KIO::Job* job) = 0;
		
		/**
		 * Changes output path. All data files should already been moved.
		 * This just modifies the datadir variable.
		 * @param outputpath New output path
		 */
		virtual void changeOutputPath(const QString & outputpath) = 0;
		
		/**
		 * Load a chunk into memory. If something goes wrong,
		 * an Error should be thrown.
		 * @param c The Chunk
		 */
		virtual void load(Chunk* c) = 0;

		/**
		 * Save a chunk to disk. If something goes wrong,
		 * an Error should be thrown.
		 * @param c The Chunk
		 */
		virtual void save(Chunk* c) = 0;
		
		/**
		 * Prepare a chunk for downloading.
		 * @param c The Chunk
		 * @return true if ok, false otherwise
		 */
		virtual bool prep(Chunk* c) = 0;
		
		/**
		 * Create all the data files to store the data.
		 */
		virtual void create() = 0;
		
		/**
		 * Close the cache file(s).
		 */
		virtual void close() = 0;
		
		/**
		 * Open the cache file(s)
		 */
		virtual void open() = 0;
		
		/// Does nothing, can be overridden to be alerted of download status changes of a TorrentFile
		virtual void downloadStatusChanged(TorrentFile*, bool) {};
		
		/**
		 * Preallocate diskspace for all files
		 * @param prealloc The thread doing the preallocation
		 */
		virtual void preallocateDiskSpace(PreallocationThread* prealloc) = 0;
		
		/// See if the download has existing files
		bool hasExistingFiles() const {return preexisting_files;}
		
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		virtual bool hasMissingFiles(QStringList & sl) = 0;

		/**
		 * Delete all data files, in case of multi file torrents
		 * empty directories should also be deleted.
		 */
		virtual void deleteDataFiles() = 0;
		
		/** 
		 * See if we are allowed to use mmap, when loading chunks.
		 * This will return false if we are close to system limits.
		 */
		static bool mappedModeAllowed();
		
		/**
		 * Get the number of bytes all the files of this torrent are currently using on disk.
		 * */
		virtual Uint64 diskUsage() = 0;
	};

}

#endif
