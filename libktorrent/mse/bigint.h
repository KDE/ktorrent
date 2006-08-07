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
#ifndef MSEBIGINT_H
#define MSEBIGINT_H

#include <qstring.h>
#include <util/constants.h>
#include <stdio.h>
#include <gmp.h>

using bt::Uint8;
using bt::Uint16;
using bt::Uint32;
using bt::Uint64;

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
		
		/**
		 * Assignment operator.
		 * @param bi The BigInt to copy
		 * @return *this
		 */
		BigInt & operator = (const BigInt & bi);
		
		/**
		 * Calculates
		 * (x ^ e) mod d 
		 * ^ is power
		 */
		static BigInt powerMod(const BigInt & x,const BigInt & e,const BigInt & d);
		
		/// Make a random BigInt
		static BigInt random();
		
		/// Export the bigint ot a buffer
		Uint32 toBuffer(Uint8* buf,Uint32 max_size) const;
		
		/// Make a BigInt out of a buffer
		static BigInt fromBuffer(const Uint8* buf,Uint32 size);
		
	private:
		mpz_t val;
	};

}

#endif
