/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef UTP_UTPPROTOCOL_H
#define UTP_UTPPROTOCOL_H

#include <QtGlobal>

namespace utp
{
	/*
	UTP header:
	
	0       4       8               16              24              32
	+-------+-------+---------------+---------------+---------------+
	| ver   | type  | extension     | connection_id                 |
	+-------+-------+---------------+---------------+---------------+
	| timestamp_microseconds                                        |
	+---------------+---------------+---------------+---------------+
	| timestamp_difference_microseconds                             |
	+---------------+---------------+---------------+---------------+
	| wnd_size                                                      |
	+---------------+---------------+---------------+---------------+
	| seq_nr                        | ack_nr                        |
	+---------------+---------------+---------------+---------------+
	*/
	
	struct Header
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		unsigned int type:4;
		unsigned int version:4;
#elif Q_BYTE_ORDER == Q_BIG_ENDIAN
		unsigned int version:4;
		unsigned int type:4;
#else
#error "Endiannes not defined"
#endif
		quint8 extension;
		quint16 connection_id;
		quint32 timestamp_microseconds;
		quint32 timestamp_difference_microseconds;
		quint32 wnd_size;
		quint16 seq_nr;
		quint16 ack_nr;
	};
	
	struct SelectiveAck
	{
		quint8 extension;
		quint8 length;
		quint32 bitmask;
	};
	
	struct ExtensionBits
	{
		quint8 extension;
		quint8 length;
		quint8 extension_bitmask[8];
	};
	
	// type field values
	const quint8 ST_DATA = 0;
	const quint8 ST_FIN = 1;
	const quint8 ST_STATE = 2;
	const quint8 ST_RESET = 3;
	const quint8 ST_SYN = 4;
	
	
	enum ConnectionState
	{
		CS_SYN_SENT,
		CS_CONNECTED
	};
	
	struct Connection
	{
		quint16 remote_connection_id;
		quint32 wnd_size;
		quint32 reply_micro;
		
		quint16 local_connection_id;
		quint32 max_window; // maximum number of bytes in flight
		quint32 cur_window; // number of bytes in flight 
		
		quint16 seq_nr;
		quint16 ack_nr;
	};
	
}

#endif // UTP_UTPPROTOCOL_H
