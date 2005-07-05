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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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
	*/
	class SHA1HashGen
	{
		Uint32 h0;
		Uint32 h1;
		Uint32 h2;
		Uint32 h3;
		Uint32 h4;
	public:
		SHA1HashGen();
		~SHA1HashGen();

		SHA1Hash generate(Uint8* data,Uint32 len);
	private:
		void processChunk(Uint8* c);
	};

}

#endif
