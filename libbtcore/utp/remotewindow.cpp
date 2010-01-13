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

namespace utp
{
	
	RemoteWindow::RemoteWindow() : cur_window(0),max_window(64 * 1024),wnd_size(0)
	{

	}

	RemoteWindow::~RemoteWindow()
	{

	}

	void RemoteWindow::packetReceived(const utp::Header* hdr)
	{
		wnd_size = hdr->wnd_size;
		
		// everything up until the ack_nr in the header should now have been acked
		QList<QByteArray>::iterator i = unacked_packets.begin();
		while (i != unacked_packets.end())
		{
			if (((utp::Header*)i->data())->seq_nr <= hdr->ack_nr)
			{
				cur_window -= i->size();
				i = unacked_packets.erase(i);
			}
			else
				break;
		}
	}

	void RemoteWindow::addPacket(const QByteArray& data)
	{
		cur_window += data.size();
		unacked_packets.append(data);
	}

}

