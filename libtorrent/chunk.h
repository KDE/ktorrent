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

#include <libutil/constants.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Keep track of a piece of the file
	 * 
	 * Keeps track of a piece of the file. The Chunk has 3 possible states :
	 * - IN_MEMORY : It's loaded into memory
	 * - ON_DISK : It's in the cache file
	 * - NOT_DOWNLOADED : It hasn't been dowloaded yet
	 */
	class Chunk
	{
	public:
		Chunk(unsigned int index,Uint32 size);
		~Chunk();

		enum Status
		{
		    IN_MEMORY,
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

		/// Set the data
		void setData(Uint8* d);

		/// Clear the chunk (delete data)
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

		/// allocate data if not allready done
		void allocate();

		/// Is chunk prioritised
		bool isPriority() const;

		/// (De)prioritise chunk
		void setPriority(bool yes);

		/// Is chunk excluded
		bool isExcluded() const;

		/// In/Exclude chunk
		void setExclude(bool yes);
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
