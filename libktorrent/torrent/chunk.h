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
#ifndef BTCHUNK_H
#define BTCHUNK_H

#include <util/constants.h>
#include <util/cachefile.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Keep track of a piece of the file
	 * 
	 * Keeps track of a piece of the file. The Chunk has 3 possible states :
	 * - MMAPPED : It is memory mapped
	 * - BUFFERED : It is in a buffer in dynamicly allocated memory 
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

		/// Is chunk prioritised
		bool isPriority() const;

		/// (De)prioritise chunk
		void setPriority(bool yes);

		/// Is chunk excluded
		bool isExcluded() const;

		/// In/Exclude chunk
		void setExclude(bool yes);
		
		virtual void unmapped(bool remap_intended);
		virtual void remapped(void* ptr);
	private:
		Status status;
		Uint32 index;
		Uint8* data;
		Uint32 size;
		int ref_count;
		bool priority;
		bool exclude;
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

	inline bool Chunk::isPriority() const {return priority;}
	inline void Chunk::setPriority(bool yes) {priority = yes;}
	inline bool Chunk::isExcluded() const {return exclude;}
	inline void Chunk::setExclude(bool yes) {exclude = yes;}
}

#endif
