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
#ifndef DHTKEY_H
#define DHTKEY_H

#include <qcstring.h>
#include <util/sha1hash.h>



namespace dht
{

	/**
	 * @author Joris Guisson
	 * @brief Key in the distributed hash table
	 *
	 * Key's in the distributed hash table are just SHA-1 hashes.
	 * Key provides all necesarry operators to be used as a value.
	 */
	class Key : public bt::SHA1Hash
	{
	public:
		/**
		 * Constructor, sets key to 0.
		 */
		Key();
		
		/**
		 * Copy constructor.
		 * @param k Key to copy
		 */
		Key(const Key & k);
		
		/**
		 * Make a key out of a bytearray
		 * @param ba The QByteArray
		 */
		Key(const QByteArray & ba);

		/// Destructor.
		virtual ~Key();
		
		/**
		 * Create a random key.
		 * @return A random Key
		 */
		static Key random();

		/**
		 * Assignment operator.
		 * @param k Key to copy
		 */
		Key & operator = (const Key & k);

		/**
		 * Equality operator.
		 * @param other The key to compare
		 * @return true if this key is equal to other
		 */
		bool operator == (const Key & other) const;

		/**
		 * Inequality operator.
		 * @param other The key to compare
		 * @return true if this key is not equal to other
		 */
		bool operator != (const Key & other) const;
		
		/**
		 * Smaller then operator.
		 * @param other The key to compare
		 * @return rue if this key is smaller then other
		 */
		bool operator < (const Key & other) const;

		
		/**
		 * Smaller then or equal operator.
		 * @param other The key to compare
		 * @return rue if this key is smaller then or equal to other
		 */
		bool operator <= (const Key & other) const;

		
		/**
		 * Greater then operator.
		 * @param other The key to compare
		 * @return rue if this key is greater then other
		 */
		bool operator > (const Key & other) const;

		/**
		 * Greater then or equal operator.
		 * @param other The key to compare
		 * @return rue if this key is greater then or equal to other
		 */
		bool operator >= (const Key & other) const;
		
		/**
		 * The distance of two keys is the keys xor together.
		 * @param a The first key
		 * @param b The second key
		 * @return a xor b
		 */
		static Key distance(const Key & a,const Key & b);
	};

}

#endif
