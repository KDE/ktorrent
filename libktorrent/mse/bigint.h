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
#ifndef MSEBIGINT_H
#define MSEBIGINT_H

#include <qstring.h>
#include <util/constants.h>

using bt::Uint8;
using bt::Uint32;

namespace mse
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Class which can hold an arbitrary large integer. This will be a very important part of our
	 * MSE implementation.
	*/
	class BigInt
	{
	public:
		/**
		 * Create a big integer, with num_bits bits.
		 * All bits will be set to 0.
		 * @param num_bits The number of bits
		 */
		BigInt(Uint32 num_bits = 0);
		
		/**
		 * Create a big integer of a string. The string must be
		 * a hexadecimal representation of an integer. For example :
		 * 12AFFE123488BBBE123
		 * 
		 * Letters can be upper or lower case. Invalid chars will create an invalid number.
		 * @param value The hexadecimal representation of the number
		 */
		BigInt(const QString & value);
		
		/**
		 * Copy constructor.
		 * @param bi BigInt to copy
		 */
		BigInt(const BigInt & bi);
		virtual ~BigInt();
		
		/// See if the number is valid.
		bool isValid() const {return data != 0;}
		
		/**
		 * Assignment operator.
		 * @param bi The BigInt to copy
		 * @return *this
		 */
		BigInt & operator = (const BigInt & bi);
		
		/**
		 * Calculate the modulo of two BigInt's.
		 * @param a The first BigInt
		 * @param b The second BigInt
		 * @return a % b
		 */
		static BigInt mod(const BigInt & a,const BigInt & b);
		
		/**
		 * Calculate the xor of two BigInt's.
		 * @param a The first BigInt
		 * @param b The second BigInt
		 * @return a ^ b
		 */
		static BigInt exclusiveOr(const BigInt & a,const BigInt & b);
		
		/// Get the number of bits
		Uint32 getNumBits() const {return num_bits;}
		
		/// Get the number of bytes
		Uint32 getNumBytes() const {return num_bytes;}
		
		/// Get the data
		const Uint8* getData() const {return data;}
		
		/// Test if the BigInt is equal to 0
		bool isNull() const;
		
		friend bool operator > (const BigInt & a,const BigInt & b);
		friend bool operator == (const BigInt & a,const BigInt & b);
		friend bool operator != (const BigInt & a,const BigInt & b);
	private:
		void hexDump() const;
	private:
		Uint8* data; // data is stored in reversed byte order (i.e. little endian)
		Uint32 num_bits,num_bytes;
	};

}

#endif
