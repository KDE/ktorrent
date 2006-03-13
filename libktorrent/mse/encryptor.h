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
#ifndef MSEENCRYPTOR_H
#define MSEENCRYPTOR_H

#include <util/array.h>
#include <util/constants.h>

using bt::Uint8;
using bt::Uint32;

namespace mse
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Interface for all encryption methods.
	*/
	class Encryptor
	{
	public:
		Encryptor();
		virtual ~Encryptor();

		virtual void encrypt(bt::Array<Uint8> & data) = 0;
		virtual void decrypt(bt::Array<Uint8> & data) = 0;
	};

}

#endif
