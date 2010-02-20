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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "dhtpeersource.h"
#include <kurl.h>
#include <qhostaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <interfaces/torrentinterface.h>
#include "dht.h"
#include "announcetask.h"

using namespace bt;

namespace dht
{

	DHTPeerSource::DHTPeerSource(DHTBase & dh_table,const bt::SHA1Hash & info_hash,const QString & torrent_name) 
		: dh_table(dh_table),curr_task(0),info_hash(info_hash),torrent_name(torrent_name)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		connect(&dh_table,SIGNAL(started()),this,SLOT(manualUpdate()));
		connect(&dh_table,SIGNAL(stopped()),this,SLOT(dhtStopped()));
		started = false;
		timer.setSingleShot(true);
		request_interval = 5 * 60 * 1000;
	}


	DHTPeerSource::~DHTPeerSource()
	{
		if (curr_task)
			curr_task->kill();
	}
	
	void DHTPeerSource::start()
	{
		started = true;
		if (dh_table.isRunning())
			doRequest();
	}
	
	void DHTPeerSource::dhtStopped()
	{
		stop(0);
		curr_task = 0;
	}
	
	void DHTPeerSource::stop(bt::WaitJob*)
	{
		started = false;
		if (curr_task)
		{
			curr_task->kill();
			timer.stop();
		}
	}
	
	void DHTPeerSource::manualUpdate()
	{
		if (dh_table.isRunning() && started)
			doRequest();
	}


	bool DHTPeerSource::doRequest()
	{
		if (!dh_table.isRunning())
			return false;
		
		if (curr_task)
			return true;
		
		Uint16 port = ServerInterface::getPort();
		curr_task = dh_table.announce(info_hash,port);
		if (curr_task)
		{
			foreach (const bt::DHTNode & n,nodes)
				curr_task->addDHTNode(n.ip,n.port);

			connect(curr_task,SIGNAL(dataReady( Task* )),this,SLOT(onDataReady( Task* )));
			connect(curr_task,SIGNAL(finished( Task* )),this,SLOT(onFinished( Task* )));
			return true;
		}
		
		return false;
	}
	
	void DHTPeerSource::onFinished(Task* t)
	{
		if (curr_task == t)
		{
			onDataReady(curr_task);
			curr_task = 0;
			timer.start(request_interval);
		}
	}
	
	void DHTPeerSource::onDataReady(Task* t)
	{
		if (curr_task == t)
		{
			Uint32 cnt = 0;
			DBItem item;
			while (curr_task->takeItem(item))
			{
				const KNetwork::KInetSocketAddress & addr = item.getAddress();
			/*	Out(SYS_DHT|LOG_NOTICE) << 
						QString("DHT: Got potential peer %1 for torrent %2")
						.arg(addr.toString()).arg(tor->getStats().torrent_name) << endl;*/
				addPeer(addr.ipAddress().toString(),addr.port());
				cnt++;
			}
			
			if (cnt)
			{
				Out(SYS_DHT|LOG_NOTICE) << 
						QString("DHT: Got %1 potential peers for torrent %2")
						.arg(cnt).arg(torrent_name) << endl;
				peersReady(this);
			}
		}
	}
	
	void DHTPeerSource::onTimeout()
	{
		if (dh_table.isRunning() && started)
			doRequest();
	}
	
	void DHTPeerSource::addDHTNode(const bt::DHTNode& node)
	{
		nodes.append(node);
	}

	void DHTPeerSource::setRequestInterval(Uint32 interval)
	{
		request_interval = interval;
	}

}

