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

#include <util/sha1hash.h>
#include "chunk.h"
#include "cache.h"
#include "piecedata.h"

namespace bt
{

	Chunk::Chunk(Uint32 index,Uint32 size,Cache* cache)
	: status(Chunk::NOT_DOWNLOADED),index(index),size(size),priority(NORMAL_PRIORITY),cache(cache)
	{
	}


	Chunk::~Chunk()
	{
	}
		
	bool Chunk::readPiece(Uint32 off,Uint32 len,Uint8* data)
	{
		PieceDataPtr d = cache->loadPiece(this,off,len);
		if (d)
			memcpy(data,d->data(),len);
		return d != 0;
	}
				
	bool Chunk::checkHash(const SHA1Hash & h)
	{
		if (status == NOT_DOWNLOADED)
			return false;
		
		PieceDataPtr d = getPiece(0,size,true);
		if (!d)
			return false;
		
		return SHA1Hash::generate(d->data(),size) == h;
	}
	
	PieceDataPtr Chunk::getPiece(Uint32 off,Uint32 len,bool read_only)
	{
		if (read_only)
			return cache->loadPiece(this,off,len);
		else
			return cache->preparePiece(this,off,len);
	}
	
	void Chunk::savePiece(PieceDataPtr piece)
	{
		cache->savePiece(piece);
	}
}
