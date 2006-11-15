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
#ifndef BTSHA1HASHGEN_H
#define BTSHA1HASHGEN_H

#include "constants.h"
#include "sha1hash.h"

namespace bt
{
	
	/**
	 * @author Joris Guisson
	 * 
	 * Generates a SHA1 hash, code based on wikipedia's pseudocode
	 * There are 2 ways to use this class :
	 * - generate : all data is present from the start
	 * - start, update and end : data can be delivered in chunks
	 * 
	 * Mixing the 2, is not a good idea
	*/
	class SHA1HashGen
	{
		Uint32 h0;
		Uint32 h1;
		Uint32 h2;
		Uint32 h3;
		Uint32 h4;
		Uint8 tmp[64];
		Uint32 tmp_len;
		Uint32 total_len;
	public:
		SHA1HashGen();
		~SHA1HashGen();

		/**
		 * Generate a hash from a bunch of data.
		 * @param data The data
		 * @param len The length
		 * @return The SHA1 hash
		 */
		SHA1Hash generate(const Uint8* data,Uint32 len);
		
		/**
		 * Start SHA1 hash generation in chunks.
		 */
		void start();
		
		/**
		 * Update the hash.
		 * @param data The data 
		 * @param len Length of the data
		 */
		void update(const Uint8* data,Uint32 len);
		
		
		/**
		 * All data has been delivered, calculate the final hash. 
		 * @return 
		 */ 
		void end();
		
		/**
		 * Get the hash generated.
		 */
		SHA1Hash get() const;
	private:
		void processChunk(const Uint8* c);
	};

}

#endif
