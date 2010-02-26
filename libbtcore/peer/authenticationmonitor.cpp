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
#include "authenticationmonitor.h"
#include <math.h>
#include <unistd.h>
#include <util/functions.h>
#include <util/log.h>
#include <mse/streamsocket.h>
#include "authenticatebase.h"
#include <kdebug.h>


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
		
		reset();
		
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
				mse::StreamSocket* socket = ab->getSocket();
				if (socket)
				{
					net::SocketDevice* dev = socket->socketDevice();
					if (dev)
					{
						net::Poll::Mode m = socket->connecting() ? Poll::OUTPUT : Poll::INPUT;
						dev->prepare(this,m);
					}
				}
				itr++;
			}
		}
		
		if (poll(50))
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
			if (!ab || ab->isFinished())
			{
				if (ab)
					ab->deleteLater();
				itr = auths.erase(itr);
			}
			else
			{
				mse::StreamSocket* socket = ab->getSocket();
				if (socket)
				{
					net::SocketDevice* dev = socket->socketDevice();
					bool r = dev && dev->ready(this,Poll::INPUT);
					bool w = dev && dev->ready(this,Poll::OUTPUT);
					if (r)
						ab->onReadyRead();
					if (w)
						ab->onReadyWrite();
				}
				itr++;
			}
		}
	}
	
}
