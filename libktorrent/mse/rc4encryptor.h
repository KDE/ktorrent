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
#ifndef MSERC4ENCRYPTOR_H
#define MSERC4ENCRYPTOR_H

#include <util/sha1hash.h>
#include "encryptor.h"

namespace mse
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		RC4 encryptor. Uses the RC4 algorithm to encrypt and decrypt data.
	*/
	class RC4Encryptor : public Encryptor
	{
		bt::SHA1Hash dkey,ekey;
		Uint8 di,dj,ei,ej;
		Uint8 ds[256];
		Uint8 es[256];
	public:
		RC4Encryptor(const bt::SHA1Hash & dkey,const bt::SHA1Hash & ekey);
		virtual ~RC4Encryptor();

		virtual void decrypt(bt::Array< Uint8 >& data);
		virtual void encrypt(bt::Array< Uint8 >& data);
		
	private:
		Uint8 prga(bool d);
	};

}

#endif
