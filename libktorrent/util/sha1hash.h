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
#ifndef BTSHA1HASH_H
#define BTSHA1HASH_H

#include <qcstring.h> 
#include "constants.h"

class QString;

namespace bt
{
	class Log;

	/**
	 * @author Joris Guisson
	 * @brief Stores a SHA1 hash
	 *
	 * This class keeps track of a SHA1 hash. A SHA1 hash is a 20 byte
	 * array of bytes.
	*/
	class SHA1Hash
	{
	protected:
		Uint8 hash[20];
	public:
		/**
		 * Constructor, sets every byte in the hash to 0.
		 */
		SHA1Hash();
		
		/**
		 * Copy constructor.
		 * @param other Hash to copy
		 */
		SHA1Hash(const SHA1Hash & other);
		
		/**
		 * Directly set the hash data.
		 * @param h The hash data must be 20 bytes large
		 */
		SHA1Hash(const Uint8* h);
		
		/**
		 * Destructor.
		 */
		virtual ~SHA1Hash();

		/// Get the idx'th byte of the hash.
		Uint8 operator [] (const Uint32 idx) const {return idx < 20 ? hash[idx] : 0;}
		
		/**
		 * Assignment operator.
		 * @param other Hash to copy
		 */
		SHA1Hash & operator = (const SHA1Hash & other);

		/**
		 * Test wether another hash is equal to this one.
		 * @param other The other hash
		 * @return true if equal, false otherwise
		 */
		bool operator == (const SHA1Hash & other) const;

		/**
		 * Test wether another hash is not equal to this one.
		 * @param other The other hash
		 * @return true if not equal, false otherwise
		 */
		bool operator != (const SHA1Hash & other) const {return !operator ==(other);}

		/**
		 * Generate an SHA1 hash from a bunch of data.
		 * @param data The data
		 * @param len Size in bytes of data
		 * @return The generated SHA1 hash
		 */
		static SHA1Hash generate(const Uint8* data,Uint32 len);

		/**
		 * Convert the hash to a printable string.
		 * @return The string
		 */
		QString toString() const;

		/**
		 * Convert the hash to a string, usable in http get requests.
		 * @return The string
		 */
		QString toURLString() const;
		
		/**
		 * Directly get pointer to the data.
		 * @return The data
		 */
		const Uint8* getData() const {return hash;}

		/**
		 * Function to print a SHA1Hash to the Log.
		 * @param out The Log
		 * @param h The hash
		 * @return out
		 */
		friend Log & operator << (Log & out,const SHA1Hash & h);
		
		
		/**
		 * XOR two SHA1Hashes
		 * @param a The first hash
		 * @param b The second
		 * @return a xor b
		 */
		friend SHA1Hash operator ^ (const SHA1Hash & a,const SHA1Hash & b);
		
		/**
		 * Function to compare 2 hashes
		 * @param a The first hash
		 * @param h The second hash
		 * @return wether a is smaller then b
		 */
		friend bool operator < (const SHA1Hash & a,const SHA1Hash & b);
		
		/**
		 * Convert the hash to a byte array.
		 */
		QByteArray toByteArray() const;
	};

}

#endif
