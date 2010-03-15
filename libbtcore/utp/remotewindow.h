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

#ifndef UTP_REMOTEWINDOW_H
#define UTP_REMOTEWINDOW_H

#include <QList>
#include <QByteArray>
#include <QMutex>
#include <btcore_export.h>
#include <util/constants.h>
#include <utp/timevalue.h>

namespace utp
{
	struct SelectiveAck;
	class Connection;
	struct Header;
	
	struct UnackedPacket
	{
		UnackedPacket(const QByteArray & data,bt::Uint16 seq_nr,bt::TimeStamp send_time);
		~UnackedPacket();
					  
		QByteArray data;
		bt::Uint16 seq_nr;
		bt::TimeStamp send_time;
		bool retransmitted;
	};
	
	/**
		The Retransmitter provides is an interface class to retransmit packets
	*/
	class BTCORE_EXPORT Retransmitter
	{
	public:
		virtual ~Retransmitter() {}
		
		/// Update the RTT time
		virtual void updateRTT(const Header* hdr,bt::Uint32 packet_rtt,bt::Uint32 packet_size) = 0;
		
		/// Retransmit a packet
		virtual void retransmit(const QByteArray & packet,bt::Uint16 p_seq_nr) = 0;
		
		/// Get the current timeout
		virtual bt::Uint32 currentTimeout() const = 0;
	};
	
	/**
		Keeps track of the remote sides window including all packets inflight.
	*/
	class BTCORE_EXPORT RemoteWindow
	{
	public:
		RemoteWindow();
		virtual ~RemoteWindow();
		
		/// A packet was received (update window size and check for acks)
		void packetReceived(const Header* hdr,const SelectiveAck* sack,Retransmitter* conn);
		
		/// Add a packet to the remote window (should include headers)
		void addPacket(const QByteArray & data,bt::Uint16 seq_nr,bt::TimeStamp send_time);
		
		/// Are we allowed to send
		bool allowedToSend(bt::Uint32 packet_size) const
		{
			return cur_window + packet_size <= qMin(wnd_size,max_window);
		}
		
		/// Calculates how much window space is availabe
		bt::Uint32 availableSpace() const
		{
			bt::Uint32 m = qMin(wnd_size,max_window);
			if (cur_window > m)
				return 0;
			else
				return m - cur_window;
		}
		
		/// See if all packets are acked
		bool allPacketsAcked() const {return unacked_packets.isEmpty();}
		
		/// Get the number of unacked packets
		bt::Uint32 numUnackedPackets() const {return unacked_packets.count();}
		
		/// A timeout occured
		void timeout(Retransmitter* conn);
		
		/// Get the window usage factor 
		double windowUsageFactor() const {return qMax((double)cur_window / max_window, 1.0);}
		
		/// Update the window size
		void updateWindowSize(double scaled_gain);
		
		bt::Uint32 currentWindow() const {return cur_window;}
		bt::Uint32 maxWindow() const {return max_window;}
		bt::Uint32 windowSize() const {return wnd_size;}
		
		/// Clear the window
		void clear();
		
	private:
		void checkLostPackets(const Header* hdr,const SelectiveAck* sack,Retransmitter* conn);
		bt::Uint16 lost(const SelectiveAck* sack);
		
	private:
		bt::Uint32 cur_window;
		bt::Uint32 max_window;
		bt::Uint32 wnd_size; // advertised window size from the other side
		QList<UnackedPacket*> unacked_packets;
		bt::Uint16 last_ack_nr;
		bt::Uint32 last_ack_receive_count;
	};

}

#endif // UTP_REMOTEWINDOW_H
