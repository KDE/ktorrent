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
#ifndef BTCACHEFILE_H
#define BTCACHEFILE_H

#include <qmap.h>
#include <qstring.h>
#include "constants.h"

namespace bt
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
		
		Used by Single and MultiFileCache to write to disk.
	*/
	class CacheFile
	{
		int fd;
		Uint64 max_size,file_size;
		QString path;
		QMap<void*,Uint32> offsetted_mappings; // mappings where offset wasn't a multiple of 4K
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
		
		/**
		 * Map a part of the file into memory, will expand the file
		 * if it is to small, but will not go past the limit set in open.
		 * @param off Offset into the file
		 * @param size Size of the region to map
		 * @param mode How the region will be mapped
		 * @return A ptr to the mmaped region, or 0 if something goes wrong
		 */
		void* map(Uint64 off,Uint32 size,Mode mode);
		
		/**
		 * Unmap a previously mapped region.
		 * @param ptr Ptr to the region
		 * @param size Size of the region
		 */
		void unmap(void* ptr,Uint32 size);
		
		/**
		 * Close the file, everything must be unmapped.
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
		
	private:
		void growFile(Uint64 to_write);
	};

}

#endif
