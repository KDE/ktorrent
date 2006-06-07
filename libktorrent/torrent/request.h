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
#ifndef BTREQUEST_H
#define BTREQUEST_H

#include "globals.h"


namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Request of a piece sent to other peers
	 *
	 * This class keeps track of a request of a piece.
	 * The Request consists of an index (the index of the chunk),
	 * offset into the chunk and the length of a piece.
	 *
	 * The PeerID of the Peer who sent the request is also kept.
	 */
	class Request
	{
	public:
		/**
		 * Constructor, set everything to 0.
		 */
		Request();
		
		/**
		 * Constructor, set the index, offset,length and peer
		 * @param index The index of the chunk 
		 * @param off The offset into the chunk
		 * @param len The length of the piece
		 * @param peer The ID of the Peer who sent the request
		 */
		Request(Uint32 index,Uint32 off,Uint32 len,Uint32 peer);
		
		/**
		 * Copy constructor.
		 * @param r Request to copy
		 */
		Request(const Request & r);
		virtual ~Request();

		/// Get the index of the chunk
		Uint32 getIndex() const {return index;}

		/// Get the offset into the chunk
		Uint32 getOffset() const {return off;}

		/// Get the length of a the piece
		Uint32 getLength() const {return len;}

		/// Get the sending Peer
		Uint32 getPeer() const {return peer;}
		
		/**
		 * Assignmenth operator.
		 * @param r The Request to copy
		 */
		Request & operator = (const Request & r);
		
		/**
		 * Compare two requests. Return true if they are the same.
		 * This only compares the index,offset and length.
		 * @param a The first request
		 * @param b The second request
		 * @return true if they are equal
		 */
		friend bool operator == (const Request & a,const Request & b);
	private:
		Uint32 index,off,len;
		Uint32 peer;
	};

}

#endif
