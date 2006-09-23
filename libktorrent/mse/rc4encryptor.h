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
#ifndef MSERC4ENCRYPTOR_H
#define MSERC4ENCRYPTOR_H

#include <util/sha1hash.h>
#include <util/constants.h>

using bt::Uint8;
using bt::Uint32;

namespace mse
{
	/**
	 * Helper class to do the actual encryption / decryption
	*/
	class RC4
	{
		Uint8 i,j;
		Uint8 s[256];
	public:
		RC4(const Uint8* key,Uint32 size);
		virtual ~RC4();
		
		void process(const Uint8* in,Uint8* out,Uint32 size);
		Uint8 process(Uint8 b);
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * RC4 encryptor. Uses the RC4 algorithm to encrypt and decrypt data.
	 * This class has a static encryption buffer, which makes it not thread safe
	 * because the buffer is not protected by mutexes.
	*/
	class RC4Encryptor
	{
		RC4 enc,dec;
	public:
		RC4Encryptor(const bt::SHA1Hash & dkey,const bt::SHA1Hash & ekey);
		virtual ~RC4Encryptor();

		/**
		 * Decrypt some data, decryption happens in place (original data gets overwritten)
		 * @param data The data
		 * @param len Size of the data
		 */
		void decrypt(Uint8* data,Uint32 len);
		
		/**
		 * Encrypt the data. Encryption happens into the static buffer.
		 * So that the data passed to this function is never overwritten.
		 * If we send pieces we point directly to the mmap region of data,
		 * this cannot be overwritten, hence the static buffer.
		 * @param data The data
		 * @param len The length of the data
		 * @return Pointer to the static buffer
		 */
		const Uint8* encrypt(const Uint8* data,Uint32 len);
		
		/**
		 * Encrypt data, encryption will happen in the same buffer. So data will 
		 * be changed replaced by it's encrypted version.
		 * @param data The data to encrypt 
		 * @param len The length of the data
		 */
		void encryptReplace(Uint8* data,Uint32 len);
		
		/**
		 * Encrypts one byte.
		 * @param b The byte to encrypt
		 * @return The encrypted byte
		 */
		Uint8 encrypt(Uint8 b) {return enc.process(b);}
	};

}

#endif
