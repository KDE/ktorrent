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
#include <btcore_export.h>

namespace utp
{
	struct Header;
	/**
		Keeps track of the remote sides window including all packets inflight.
	*/
	class BTCORE_EXPORT RemoteWindow
	{
	public:
		RemoteWindow();
		virtual ~RemoteWindow();
		
		/// A packet was received (update window size and check for acks)
		void packetReceived(const Header* hdr);
		
		/// Add a packet to the remote window (should include headers)
		void addPacket(const QByteArray & data);
		
		/// Are we allowed to send
		bool allowedToSend(quint32 packet_size) const
		{
			return cur_window + packet_size <= qMin(wnd_size,max_window);
		}
		
	private:
		quint32 cur_window;
		quint32 max_window;
		quint32 wnd_size; // advertised window size from the other side
		QList<QByteArray> unacked_packets;
	};

}

#endif // UTP_REMOTEWINDOW_H
