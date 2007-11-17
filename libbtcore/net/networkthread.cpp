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
#include <util/functions.h>
#include <util/log.h>
#include "socketgroup.h"
#include "socketmonitor.h"
#include "networkthread.h"
		
using namespace bt;

namespace net
{

	NetworkThread::NetworkThread(SocketMonitor* sm)
		: sm(sm),running(false)
	{
		groups.setAutoDelete(true);
		groups.insert(0,new SocketGroup(0));
	}


	NetworkThread::~NetworkThread()
	{}

	void NetworkThread::run()
	{
		running = true;
		prev_run_time = bt::Now();
		while (running)
			update();
	}

	void NetworkThread::addGroup(Uint32 gid,Uint32 limit)
	{
		// if group already exists, just change the limit
		SocketGroup* g = groups.find(gid);
		if (g)
		{
			g->setLimit(limit);
		}
		else
		{
			g = new SocketGroup(limit);
			groups.insert(gid,g);
		}
	}
		
	void NetworkThread::removeGroup(Uint32 gid)
	{
		// make sure the 0 group is never erased
		if (gid != 0)
			groups.erase(gid);
	}
		
	void NetworkThread::setGroupLimit(Uint32 gid,Uint32 limit)
	{
		SocketGroup* g = groups.find(gid);
		if (g)
		{
			g->setLimit(limit);
		}
	}
	
	Uint32 NetworkThread::doGroupsLimited(Uint32 num_ready,bt::TimeStamp now,Uint32 & allowance)
	{
		Uint32 num_still_ready = 0;
		
		// this is one pass over all the groups
		bt::PtrMap<Uint32,SocketGroup>::iterator itr = groups.begin();
		while (itr != groups.end() && allowance > 0)
		{
			SocketGroup* g = itr->second;
			if (g->numSockets() > 0)
			{
				Uint32 group_allowance = (Uint32)ceil(((double)g->numSockets() / num_ready) * allowance);
				
				// lets not do to much and make sure we don't pass 0 to the socket group (0 is unlimited)
				if (group_allowance > allowance || group_allowance == 0)
					group_allowance = allowance;  
				
				Uint32 ga = group_allowance;
				
				if (!doGroup(g,ga,now))
					g->clear(); // group is done, so clear it
				else
					num_still_ready += g->numSockets(); // keep track of the number of sockets which are still ready
					
				Uint32 done = group_allowance - ga;
				if (allowance >= done)
					allowance -= done;
				else
					allowance = 0;
			}
			itr++;
		}
		
		return num_still_ready > 0;
	}
	
	void NetworkThread::doGroups(Uint32 num_ready,bt::TimeStamp now,bt::Uint32 limit)
	{
		if (limit == 0)
		{
			Uint32 allowance = 0;
			bt::PtrMap<Uint32,SocketGroup>::iterator itr = groups.begin();
			while (itr != groups.end())
			{
				SocketGroup* g = itr->second;
				if (g->numSockets() > 0)
				{
					g->calcAllowance(now);
					doGroup(g,allowance,now);
					g->clear();
				}
				itr++;
			}
		}
		else
		{
			// calculate group allowance for each group
			bt::PtrMap<Uint32,SocketGroup>::iterator itr = groups.begin();
			while (itr != groups.end())
			{
				SocketGroup* g = itr->second;
				g->calcAllowance(now);
				itr++;
			}
			
			Uint32 allowance = (Uint32)ceil(1.02 * limit * (now - prev_run_time) * 0.001);
			
			while (allowance > 0 && num_ready > 0)
			{
				// loop until nobody is ready anymore or the allowance is up
				num_ready = doGroupsLimited(num_ready,now,allowance);
			}
			
			// make sure all groups are cleared
			itr = groups.begin();
			while (itr != groups.end())
			{
				SocketGroup* g = itr->second;
				g->clear();
				itr++;
			}
		}
	}
}
