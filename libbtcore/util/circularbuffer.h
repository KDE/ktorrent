/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef BT_CIRCULARBUFFER_H
#define BT_CIRCULARBUFFER_H

#include <btcore_export.h>
#include <util/constants.h>
#include <boost/circular_buffer.hpp>

namespace bt
{
	
	/**
		Circular buffer class
	*/
	class BTCORE_EXPORT CircularBuffer : public boost::circular_buffer<bt::Uint8>
	{
	public:
		CircularBuffer(bt::Uint32 cap = 64 * 1024);
		virtual ~CircularBuffer();
		
		/**
			Read up to max_len bytes from the buffer and store it in data
			@param data The place to store the data
			@param max_len Maximum amount to read
			@return The amount read
		*/
		virtual bt::Uint32 read(bt::Uint8* data,bt::Uint32 max_len);
		
		/**
			Write up to len bytes from data and store it in the window.
			@param data The data to copy
			@param max_len Amount to write
			@return The amount written
		*/
		virtual bt::Uint32 write(const bt::Uint8* data,bt::Uint32 len);
	};

}

#endif // BT_CIRCULARBUFFER_H
