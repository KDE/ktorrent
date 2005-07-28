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
		Chunk(unsigned int index,unsigned int size);
		~Chunk();
		
		enum Status 
		{
			IN_MEMORY,
			ON_DISK,
			NOT_DOWNLOADED
		};

		Status getStatus() const {return status;}
		void setStatus(Status s) {status = s;}
		
		const unsigned char* getData() const {return data;}
		unsigned char* getData() {return data;}
		
		void setData(unsigned char* d);
		void clear();
		
		unsigned int getIndex() const {return index;}
		unsigned int getSize() const {return size;}
		
		void ref() {ref_count++;}
		void unref() {ref_count--;}
		bool taken() const {return ref_count > 0;}
		
		unsigned int getCacheFileOffset() const {return cache_file_offset;}
		void setCacheFileOffset(unsigned int off) {cache_file_offset = off;}
		
		void allocate();

		bool isPriority() const {return priority;}
		void setPriority(bool yes) {priority = yes;}
		bool isExcluded() const {return exclude;}
		void setExclude(bool yes) {exclude = yes;}
	private:
		Status status;
		unsigned int index;
		unsigned char* data;
		unsigned int size;
		int ref_count;
		unsigned int cache_file_offset;
		bool priority;
		bool exclude;
	};

}

#endif
