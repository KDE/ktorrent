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


#ifndef UTP_LOCALWINDOW_H
#define UTP_LOCALWINDOW_H

#include <btcore_export.h>
#include <util/constants.h>
#include <util/circularbuffer.h>
#include <QLinkedList>
#include <QByteArray>

namespace utp
{
	struct SelectiveAck;
	struct Header;
	
	const bt::Uint32 DEFAULT_CAPACITY = 64*1024;
	
	struct FuturePacket
	{
		FuturePacket(bt::Uint16 seq_nr,const bt::Uint8* data,bt::Uint32 size);
		~FuturePacket();
		
		bt::Uint16 seq_nr;
		QByteArray data;
	};
	
	/**
		Manages the local window of a UTP connection.
		This is a circular buffer.
	*/
	class BTCORE_EXPORT LocalWindow : public bt::CircularBuffer
	{
	public:
		LocalWindow(bt::Uint32 cap = DEFAULT_CAPACITY);
		virtual ~LocalWindow();
		
		/// Get back the available space
		bt::Uint32 availableSpace() const {return window_space;}
		
		/// Get back how large the window is
		bt::Uint32 currentWindow() const {return capacity() - window_space;}
	
		/// A packet was received
		bool packetReceived(const Header* hdr,const bt::Uint8* data,bt::Uint32 size);
		
		/// Set the last sequence number
		void setLastSeqNr(bt::Uint16 lsn);
		
		/// Get the last sequence number we can safely ack
		bt::Uint16 lastSeqNr() const {return last_seq_nr;}
		
		/// Is the window empty
		bool isEmpty() const {return future_packets.isEmpty() && empty();}
		
		virtual bt::Uint32 read(bt::Uint8* data,bt::Uint32 max_len);
		
		/// Get the number of selective ack bits needed when sending a packet
		bt::Uint32 selectiveAckBits() const;
		
		/// Fill a SelectiveAck structure
		void fillSelectiveAck(SelectiveAck* sack);
		
	private:
		void checkFuturePackets();
		
	private:
		bt::Uint16 last_seq_nr;
		// all the packets which have been received but we can yet write to the output buffer
		// due to missing packets
		QLinkedList<FuturePacket*> future_packets;
		bt::Uint32 window_space;
	};

}

#endif // UTP_LOCALWINDOW_H
