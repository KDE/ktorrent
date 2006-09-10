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
#include <sys/select.h>
#include <util/functions.h>
#include <util/log.h>
#include <mse/streamsocket.h>
#include "authenticationmonitor.h"
#include "authenticatebase.h"

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
		std::set<AuthenticateBase*>::iterator itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			ab->deleteLater();
			itr++;
		}
		auths.clear();
	}


	void AuthenticationMonitor::add(AuthenticateBase* s)
	{
		auths.insert(s);
	}
	
	void AuthenticationMonitor::remove(AuthenticateBase* s)
	{
		auths.erase(s);
	}
	
	void AuthenticationMonitor::update()
	{
		if (auths.size() == 0)
			return;
			
		fd_set rfds,wfds;
		int max_fd = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		
		std::set<AuthenticateBase*>::iterator itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			if (!ab || ab->isFinished())
			{
				if (ab)
					ab->deleteLater();
				
				std::set<AuthenticateBase*>::iterator j = itr;
				itr++;
				auths.erase(j);
			}
			else
			{
				if (ab->getSocket() && ab->getSocket()->fd() >= 0)
				{
					int fd = ab->getSocket()->fd();
					if (!ab->getSocket()->connecting())
						FD_SET(fd,&rfds);
					else
						FD_SET(fd,&wfds);
					
					if (fd > max_fd)
						max_fd = fd;
				}
				itr++;
			}
		}
		
		struct timeval tv = {0,1000};
		if (select(max_fd+1,&rfds,&wfds,NULL,&tv) <= 0)
			return;
		
		itr = auths.begin();
		while (itr != auths.end())
		{
			AuthenticateBase* ab = *itr;
			if (ab->getSocket() && ab->getSocket()->fd() >= 0)
			{
				int fd = ab->getSocket()->fd();
				if (FD_ISSET(fd,&rfds))
				{
					ab->onReadyRead();
				}
				else if (FD_ISSET(fd,&wfds))
				{
					ab->onReadyWrite();
				}
			}
			
			if (ab->isFinished())
			{
				ab->deleteLater();
				std::set<AuthenticateBase*>::iterator j = itr;
				itr++;
				auths.erase(j);
			}
			else
				itr++;
		}
	}
	
}
