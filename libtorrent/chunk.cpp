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
#include "chunk.h"
#include "globals.h"

namespace bt
{

	Chunk::Chunk(unsigned int index,unsigned int size)
	: status(Chunk::NOT_DOWNLOADED),index(index),
	data(0),size(size),ref_count(0),cache_file_offset(0)
	{}


	Chunk::~Chunk()
	{
		delete [] data;
	}

	void Chunk::setData(unsigned char* d)
	{
		status = Chunk::IN_MEMORY;
		if (data)
			delete [] data;
		data = d;
	}
	
	void Chunk::allocate()
	{
		if (data) return;
		data = new Uint8[size];
	}

	void Chunk::clear()
	{
		if (data)
		{
			status = Chunk::ON_DISK;
			delete [] data;
			data = 0;
		}
	}
}
