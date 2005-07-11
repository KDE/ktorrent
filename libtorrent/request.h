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
#ifndef BTREQUEST_H
#define BTREQUEST_H

#include "globals.h"

namespace bt
{
	class Peer;

	/**
	@author Joris Guisson
	*/
	class Request
	{
	public:
		Request();
		Request(Uint32 index,Uint32 off,Uint32 len,Peer* peer);
		Request(const Request & r);
		virtual ~Request();
		
		Uint32 getIndex() const {return index;}
		Uint32 getOffset() const {return off;}
		Uint32 getLength() const {return len;}
		const Peer* getPeer() const {return peer;}
		Peer* getPeer() {return peer;}
		
		Request & operator = (const Request & r);
		
		friend bool operator == (const Request & a,const Request & b);
	private:
		Uint32 index,off,len;
		Peer* peer;
	};

}

#endif
