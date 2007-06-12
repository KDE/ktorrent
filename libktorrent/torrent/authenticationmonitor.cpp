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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <math.h>
#include <unistd.h>
#include <sys/poll.h>
#include <util/functions.h>
#include <util/log.h>
#include <mse/streamsocket.h>
#include "authenticationmonitor.h"
#include "authenticatebase.h"

#include <util/profiler.h>


namespace bt
{
	AuthenticationMonitor AuthenticationMonitor::self;

	AuthenticationMonitor::AuthenticationMonitor()
	{}


	AuthenticationMonitor::~AuthenticationMonitor()
	{
		
	}
	
	void AuthenticationMonitor::clear()
	{
		std::list<AuthenticateBase*>::iterator itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			if (ab)
				ab->deleteLater();
			itr++;
		}
		auths.clear();
	}


	void AuthenticationMonitor::add(AuthenticateBase* s)
	{
		auths.push_back(s);
	}
	
	void AuthenticationMonitor::remove(AuthenticateBase* s)
	{
		auths.remove(s);
	}
	
	void AuthenticationMonitor::update()
	{
		if (auths.size() == 0)
			return;
		
		int i = 0;
		
		std::list<AuthenticateBase*>::iterator itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			if (!ab || ab->isFinished())
			{
				if (ab)
					ab->deleteLater();
				
				itr = auths.erase(itr);
			}
			else
			{
				ab->setPollIndex(-1);
				if (ab->getSocket() && ab->getSocket()->fd() >= 0)
				{
					int fd = ab->getSocket()->fd();
					if (i >= fd_vec.size())
					{
						struct pollfd pfd = {-1,0,0};
						fd_vec.push_back(pfd);
					}
					
					struct pollfd & pfd = fd_vec[i];
					pfd.fd = fd;
					pfd.revents = 0;
					if (!ab->getSocket()->connecting())
						pfd.events = POLLIN;
					else
						pfd.events = POLLOUT;
					ab->setPollIndex(i);
					i++;
				}
				itr++;
			}
		}
		
		if (poll(&fd_vec[0],i,1) > 0)
		{
			handleData();
		}
	}
	
	void AuthenticationMonitor::handleData()
	{
		std::list<AuthenticateBase*>::iterator itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			if (ab && ab->getSocket() && ab->getSocket()->fd() >= 0 && ab->getPollIndex() >= 0)
			{
				int pi = ab->getPollIndex();
				if (fd_vec[pi].revents & POLLIN)
				{
					ab->onReadyRead();
				}
				else if (fd_vec[pi].revents & POLLOUT)
				{
					ab->onReadyWrite();
				}
			}
			
			if (!ab || ab->isFinished())
			{
				if (ab)
					ab->deleteLater();
				itr = auths.erase(itr);
			}
			else
				itr++;
		}
	}
	
}
