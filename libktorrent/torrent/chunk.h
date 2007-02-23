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
#ifndef BTCHUNK_H
#define BTCHUNK_H

#include <util/constants.h>
#include "cachefile.h"

namespace bt
{
	class SHA1Hash;

	/**
	 * @author Joris Guisson
	 * @brief Keep track of a piece of the file
	 * 
	 * Keeps track of a piece of the file. The Chunk has 3 possible states :
	 * - MMAPPED : It is memory mapped
	 * - BUFFERED : It is in a buffer in dynamically allocated memory 
	 *  (because the chunk is located in 2 or more separate files, so we cannot just set a pointer
	 *   to a region of mmapped memory)
	 * - ON_DISK : On disk
	 * - NOT_DOWNLOADED : It hasn't been dowloaded yet, and there is no buffer allocated
	 */
	class Chunk : public MMappeable
	{
	public:
		Chunk(unsigned int index,Uint32 size);
		~Chunk();

		enum Status
		{
			MMAPPED,
			BUFFERED,
			ON_DISK,
			NOT_DOWNLOADED
		};

		/// Get the chunks status.
		Status getStatus() const;

		/**
		 * Set the chunks status
		 * @param s 
		 */
		void setStatus(Status s);

		/// Get the data
		const Uint8* getData() const;

		/// Get the data
		Uint8* getData();

		/// Set the data and the new status
		void setData(Uint8* d,Status nstatus);

		/// Clear the chunk (delete data depending on the mode)
		void clear();

		/// Get the chunk's index
		Uint32 getIndex() const;

		/// Get the chunk's size
		Uint32 getSize() const;

		/// Add one to the reference counter
		void ref();

		/// --reference counter
		void unref();

		/// reference coun > 0
		bool taken() const;

		/// allocate data if not already done, sets the status to buffered
		void allocate();

		/// get chunk priority
		Priority getPriority() const;

		/// set chunk priority
		void setPriority(Priority newpriority = NORMAL_PRIORITY);

		/// Is chunk excluded
		bool isExcluded() const;
		
		/// Is this a seed only chunk
		bool isExcludedForDownloading() const;

		/// In/Exclude chunk
		void setExclude(bool yes);
		
		/**
		 * Check wehter the chunk matches it's hash.
		 * @param h The hash
		 * @return true if the data matches the hash
		 */
		bool checkHash(const SHA1Hash & h) const;
		
	private:
		virtual void unmapped();
		
	private:
		Status status;
		Uint32 index;
		Uint8* data;
		Uint32 size;
		int ref_count;
		Priority priority;
	};

	inline Chunk::Status Chunk::getStatus() const
	{
		return status;
	}

	inline void Chunk::setStatus(Chunk::Status s)
	{
		status = s;
	}

	inline const Uint8* Chunk::getData() const {return data;}
	inline Uint8* Chunk::getData() {return data;}

	inline Uint32 Chunk::getIndex() const {return index;}
	inline Uint32 Chunk::getSize() const {return size;}

	inline void Chunk::ref() {ref_count++;}
	inline void Chunk::unref() {ref_count--;}
	inline bool Chunk::taken() const {return ref_count > 0;}

	inline Priority Chunk::getPriority() const {return priority;}
	inline void Chunk::setPriority(Priority newpriority) {priority = newpriority;}
	inline bool Chunk::isExcluded() const 
	{
		return priority == EXCLUDED; 
	}
	
	inline bool Chunk::isExcludedForDownloading() const
	{
		return priority == ONLY_SEED_PRIORITY;
	}
	
	inline void Chunk::setExclude(bool yes)
		{if(yes) priority = EXCLUDED; else priority = NORMAL_PRIORITY;}
}

#endif
