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
#include "request.h"

namespace bt
{
	Request::Request() : index(0),off(0),len(0),peer(0)
	{}

	Request::Request(Uint32 index,Uint32 off,Uint32 len,Uint32 peer)
	: index(index),off(off),len(len),peer(peer)
	{}

	Request::Request(const Request & r)
	: index(r.index),off(r.off),len(r.len),peer(r.peer)
	{}

	Request::~Request()
	{}


	Request & Request::operator = (const Request & r)
	{
		index = r.index;
		off = r.off;
		len = r.len;
		peer = r.peer;
		return *this;
	}
	
	bool operator == (const Request & a,const Request & b)
	{
		return a.index == b.index && a.len == b.len && a.off == b.off;
	}
}
