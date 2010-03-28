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

#ifndef UTP_POLLPIPE_H
#define UTP_POLLPIPE_H

#include <QSet>
#include <net/poll.h>
#include <net/wakeuppipe.h>


namespace utp
{
	class Connection;
	
	/**
		Special wake up pipe for UTP polling
	*/
	class PollPipe : public net::WakeUpPipe
	{
	public:
		PollPipe(net::Poll::Mode mode);
		virtual ~PollPipe();
		
		/// Is the pipe being polled
		bool polling() const {return poll_index >= 0;}
		
		/// Prepare the poll
		void prepare(net::Poll* p,bt::Uint16 conn_id);
		
		/// Is this connection ready to wake up
		bool readyToWakeUp(Connection* conn) const;
		
		/// Reset the poll_index
		virtual void reset();
		
		/// Polling mode
		net::Poll::Mode pollingMode() const {return mode;}
		
	private:
		net::Poll::Mode mode;
		int poll_index;
		QSet<bt::Uint16> conn_ids;
	};
}

#endif // UTP_POLLPIPE_H
