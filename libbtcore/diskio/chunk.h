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

#include <btcore_export.h>
#include <util/constants.h>

namespace bt
{
	class SHA1Hash;
	class Cache;
	class PieceData;
	class PieceDataPtr;

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
	class BTCORE_EXPORT Chunk
	{
	public:
		Chunk(Uint32 index,Uint32 size,Cache* cache);
		~Chunk();

		enum Status
		{
			ON_DISK,
			NOT_DOWNLOADED
		};
		
		/**
		 * Read a piece from the chunk
		 * @param off The offset of the chunk	
		 * @param len The length of the chunk
		 * @param data The data, should be big enough to hold len bytes
		 */
		bool readPiece(Uint32 off,Uint32 len,Uint8* data);
		
		/**
		 * Get a pointer to the data of a piece.
		 * If it isn't loaded, it will be loaded.
		 * @param off Offset of the piece
		 * @param len Length of the piece
		 * @param read_only Is this for reading the piece or for writing
		 * @return Pointer to the PieceData
		 */
		PieceDataPtr getPiece(Uint32 off,Uint32 len,bool read_only);
		
		/**
		 * Save a piece
		 * @param off Offset of the piece
		 * @param len Length of the piece
		 */
		void savePiece(PieceDataPtr piece);

		/// Get the chunks status.
		Status getStatus() const {return status;}

		/**
		 * Set the chunks status
		 * @param s 
		 */
		void setStatus(Status s) {status = s;}

		/// Get the chunk's index
		Uint32 getIndex() const {return index;}

		/// Get the chunk's size
		Uint32 getSize() const {return size;}
		
		/// get chunk priority
		Priority getPriority() const {return priority;}

		/// set chunk priority
		void setPriority(Priority newpriority = NORMAL_PRIORITY) {priority = newpriority;}

		/// Is chunk excluded
		bool isExcluded() const {return priority == EXCLUDED;}
		
		/// Is this a seed only chunk
		bool isExcludedForDownloading() const {return priority == ONLY_SEED_PRIORITY;}

		/// In/Exclude chunk
		void setExclude(bool yes) {priority = yes ? EXCLUDED : NORMAL_PRIORITY;}
		
		/**
		 * Check wehter the chunk matches it's hash.
		 * @param h The hash
		 * @return true if the data matches the hash
		 */
		bool checkHash(const SHA1Hash & h);
		
	private:
		Status status;
		Uint32 index;
		Uint32 size;
		Priority priority;
		Cache* cache;
	};
}

#endif
