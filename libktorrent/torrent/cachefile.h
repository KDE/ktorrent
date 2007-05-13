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
#ifndef BTCACHEFILE_H
#define BTCACHEFILE_H

#include <qmap.h>
#include <qmutex.h>
#include <qstring.h>
#include <util/constants.h>

namespace bt
{
	class PreallocationThread;

	
	/**
	 * Interface which classes must implement to be able to map something from a CacheFile
	 * It will also be used to notify when things get unmapped or remapped
	*/
	class MMappeable
	{
	public:
		virtual ~MMappeable() {}

		/**
		 * When a CacheFile is closed, this will be called on all existing mappings.
		 */
		virtual void unmapped() = 0;
	};

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
		
		Used by Single and MultiFileCache to write to disk.
	*/
	class CacheFile
	{
	public:
		CacheFile();
		virtual ~CacheFile();
		
		enum Mode
		{
			READ,WRITE,RW
		};

		
		/**
		 * Open the file.
		 * @param path Path of the file
		 * @param size Max size of the file
		 * @throw Error when something goes wrong
		 */
		void open(const QString & path,Uint64 size);
		
		/// Change the path of the file
		void changePath(const QString & npath);
		
		/**
		 * Map a part of the file into memory, will expand the file
		 * if it is to small, but will not go past the limit set in open.
		 * @param thing The thing that wishes to map the mmapping
		 * @param off Offset into the file
		 * @param size Size of the region to map
		 * @param mode How the region will be mapped
		 * @return A ptr to the mmaped region, or 0 if something goes wrong
		 */
		void* map(MMappeable* thing,Uint64 off,Uint32 size,Mode mode);
		
		/**
		 * Unmap a previously mapped region.
		 * @param ptr Ptr to the region
		 * @param size Size of the region
		 */
		void unmap(void* ptr,Uint32 size);
		
		/**
		 * Close the file, everything will be unmapped.
		 * @param to_be_reopened Indicates if the close is temporarely (i.e. it will be reopened)
		 */
		void close();
		
		/**
		 * Read from the file.
		 * @param buf Buffer to store data
		 * @param size Size to read
		 * @param off Offset to read from in file
		 */
		void read(Uint8* buf,Uint32 size,Uint64 off);
		
		/**
		 * Write to the file.
		 * @param buf Buffer to write
		 * @param size Size to read
		 * @param off Offset to read from in file
		 */
		void write(const Uint8* buf,Uint32 size,Uint64 off);
		
		/**
		 * Preallocate disk space
		 */
		void preallocate(PreallocationThread* prealloc);

		/// Get the number of bytes this cache file is taking up
		Uint64 diskUsage();
		
	private:
		void growFile(Uint64 to_write);
		void closeTemporary();
		void openFile(Mode mode);
		
	private:
		int fd;
		bool read_only;
		Uint64 max_size,file_size;
		QString path;
		struct Entry
		{
			MMappeable* thing;
			void* ptr;
			Uint32 size;
			Uint64 offset;
			Uint32 diff;
			Mode mode;
		};
		QMap<void*,Entry> mappings; // mappings where offset wasn't a multiple of 4K
		mutable QMutex mutex;
	};

}

#endif
