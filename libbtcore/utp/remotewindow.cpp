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

#include "remotewindow.h"
#include "utpprotocol.h"
#include "connection.h"

namespace utp
{
	
	UnackedPacket::UnackedPacket(const QByteArray& data, bt::Uint16 seq_nr, const TimeValue& send_time) 
		: data(data),seq_nr(seq_nr),send_time(send_time)
	{
	}

	UnackedPacket::~UnackedPacket()
	{
	}

	
	RemoteWindow::RemoteWindow() : cur_window(0),max_window(64 * 1024),wnd_size(0)
	{

	}

	RemoteWindow::~RemoteWindow()
	{
		qDeleteAll(unacked_packets);
	}

	void RemoteWindow::packetReceived(const utp::Header* hdr,Connection* conn)
	{
		wnd_size = hdr->wnd_size;
		
		TimeValue now;
		// everything up until the ack_nr in the header should now have been acked
		QList<UnackedPacket*>::iterator i = unacked_packets.begin();
		while (i != unacked_packets.end())
		{
			UnackedPacket* up = *i;
			if (up->seq_nr <= hdr->ack_nr)
			{
				conn->updateRTT(hdr,now - up->send_time);
				cur_window -= up->data.size();
				delete up;
				i = unacked_packets.erase(i);
			}
			else
				break;
		}
	}

	void RemoteWindow::addPacket(const QByteArray& data,bt::Uint16 seq_nr,const TimeValue & send_time)
	{
		cur_window += data.size();
		unacked_packets.append(new UnackedPacket(data,seq_nr,send_time));
	}

}

