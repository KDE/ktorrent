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
#ifndef BTCONSTANTS_H
#define BTCONSTANTS_H

namespace bt
{
	typedef unsigned long long Uint64;
	typedef unsigned long Uint32;
	typedef unsigned short Uint16;
	typedef unsigned char Uint8;

	typedef long long Int64;
	typedef long Int32;
	typedef short Int16;
	typedef char Int8;
	
	const Uint32 MAX_MSGLEN = 9 + 131072;
	const Uint16 MIN_PORT = 6881;
	const Uint16 MAX_PORT = 6889;
	const Uint32 MAX_PIECE_LEN = 16384;
	
	const Uint8 CHOKE = 0;
	const Uint8 UNCHOKE = 1;
	const Uint8 INTERESTED = 2;
	const Uint8 NOT_INTERESTED = 3;
	const Uint8 HAVE = 4;
	const Uint8 BITFIELD = 5;
	const Uint8 REQUEST = 6;
	const Uint8 PIECE = 7;
	const Uint8 CANCEL = 8;
	const Uint8 PORT = 9;
}


#endif
