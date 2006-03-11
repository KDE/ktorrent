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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kurl.h>
#include <util/functions.h>
#include <torrent/peermanager.h>
#include "dhttrackerbackend.h"
#include "dht.h"
#include "announcetask.h"

using namespace bt;

namespace dht
{

	DHTTrackerBackend::DHTTrackerBackend(Tracker* trk,DHT & dh_table)
	: TrackerBackend(trk),dh_table(dh_table),curr_task(0)
	{}


	DHTTrackerBackend::~DHTTrackerBackend()
	{}


	void DHTTrackerBackend::doRequest(const KURL& url)
	{
		if (curr_task)
			return;
		
		curr_task = dh_table.announce(frontend->info_hash,url.port());
		if (curr_task)
			curr_task->setListener(this);
	}

	void DHTTrackerBackend::updateData(PeerManager* pman)
	{
		if (!curr_task)
			return;
		
		DBItem item;
		while (curr_task->takeItem(item))
		{
			bt::PotentialPeer pp;
			pp.port = bt::ReadUint16(item.getData(),4);
			pp.ip = bt::ReadUint32(item.getData(),0);
			pman->addPotentialPeer(pp);
		}
	}
	
	void DHTTrackerBackend::onFinished(Task* t)
	{
		if (curr_task == t)
		{
			frontend->emitDataReady();
			curr_task = 0;
		}
	}
	
	void DHTTrackerBackend::onDataReady(Task* t)
	{
		if (curr_task == t)
		{
			frontend->emitDataReady();
		}
	}
}
