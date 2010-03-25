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

#include <util/log.h>
#include "pollpipe.h"
#include "connection.h"

using namespace bt;

namespace utp
{
	
	PollPipe::PollPipe(net::Poll::Mode mode) : mode(mode),poll_index(-1)
	{
	}
		
	PollPipe::~PollPipe()
	{
	}

	void PollPipe::prepare(net::Poll* p, bt::Uint16 conn_id)
	{
		QMutexLocker lock(&mutex);
		conn_ids.insert(conn_id);
		if (poll_index < 0)
			poll_index = p->add(this);
	}


	bool PollPipe::readyToWakeUp(Connection* conn) const
	{
		QMutexLocker lock(&mutex);
		if (poll_index < 0 || !conn_ids.contains(conn->receiveConnectionID()))
			return false;
		
		if (mode == net::Poll::INPUT)
			return conn->bytesAvailable() > 0 || conn->connectionState() == CS_CLOSED;
		else
			return conn->isWriteable();
	}

	void PollPipe::reset()
	{
		QMutexLocker lock(&mutex);
		poll_index = -1;
		conn_ids.clear();
	}

}

