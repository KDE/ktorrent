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
#include <kurl.h>
#include <qhostaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <torrent/peermanager.h>
#include <interfaces/torrentinterface.h>
#include "dhttrackerbackend.h"
#include "dht.h"
#include "announcetask.h"

using namespace bt;

namespace dht
{

	DHTTrackerBackend::DHTTrackerBackend(DHTBase & dh_table,kt::TorrentInterface* tor) 
		: dh_table(dh_table),curr_task(0),tor(tor)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		connect(&dh_table,SIGNAL(started()),this,SLOT(manualUpdate()));
		connect(&dh_table,SIGNAL(stopped()),this,SLOT(dhtStopped()));
		started = false;
	}


	DHTTrackerBackend::~DHTTrackerBackend()
	{
		if (curr_task)
			curr_task->kill();
	}
	
	void DHTTrackerBackend::start()
	{
		started = true;
		if (dh_table.isRunning())
			doRequest();
	}
	
	void DHTTrackerBackend::dhtStopped()
	{
		stop(0);
		curr_task = 0;
	}
	
	void DHTTrackerBackend::stop(bt::WaitJob*)
	{
		started = false;
		if (curr_task)
		{
			curr_task->kill();
			timer.stop();
		}
	}
	
	void DHTTrackerBackend::manualUpdate()
	{
		if (dh_table.isRunning() && started)
			doRequest();
	}


	bool DHTTrackerBackend::doRequest()
	{
		if (!dh_table.isRunning())
			return false;
		
		if (curr_task)
			return true;
		
		const SHA1Hash & info_hash = tor->getInfoHash();
		Uint16 port = bt::Globals::instance().getServer().getPortInUse();
		curr_task = dh_table.announce(info_hash,port);
		if (curr_task)
		{
			for (Uint32 i = 0;i < tor->getNumDHTNodes();i++)
			{
				const kt::DHTNode & n = tor->getDHTNode(i);
				curr_task->addDHTNode(n.ip,n.port);
			}
			connect(curr_task,SIGNAL(dataReady( Task* )),this,SLOT(onDataReady( Task* )));
			connect(curr_task,SIGNAL(finished( Task* )),this,SLOT(onFinished( Task* )));

			return true;
		}
		
		return false;
	}
	
	void DHTTrackerBackend::onFinished(Task* t)
	{
		if (curr_task == t)
		{
			onDataReady(curr_task);
			curr_task = 0;
			// do another announce in 5 minutes or so
			timer.start(5 * 60 * 1000,true);
		}
	}
	
	void DHTTrackerBackend::onDataReady(Task* t)
	{
		if (curr_task == t)
		{
			Uint32 cnt = 0;
			DBItem item;
			while (curr_task->takeItem(item))
			{
				Uint16 port = bt::ReadUint16(item.getData(),4);
				QString ip = QHostAddress(ReadUint32(item.getData(),0)).toString();
				
				addPeer(ip,port);
				cnt++;
			}
			
			if (cnt)
			{
				Out(SYS_DHT|LOG_NOTICE) << 
						QString("DHT: Got %1 potential peers for torrent %2")
						.arg(cnt).arg(tor->getStats().torrent_name) << endl;
				peersReady(this);
			}
		}
	}
	
	void DHTTrackerBackend::onTimeout()
	{
		if (dh_table.isRunning() && started)
			doRequest();
	}
	
}

#include "dhttrackerbackend.moc"
