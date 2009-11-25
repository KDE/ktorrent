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
#include "peersourcemanager.h"
#include <qfile.h>
#include <qtextstream.h>
#include <klocale.h>
#include <QtAlgorithms>
// #include <functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <dht/dhtbase.h>
#include <dht/dhtpeersource.h>
#include <tracker/tracker.h>
#include "torrentcontrol.h"
#include "torrent.h"
#include <peer/peermanager.h>

namespace bt
{


	PeerSourceManager::PeerSourceManager(TorrentControl* tor,PeerManager* pman) 
		: TrackerManager(tor,pman),m_dht(0)
	{
		
	}
	
	PeerSourceManager::~PeerSourceManager()
	{
		QList<PeerSource*>::iterator itr = additional.begin();
		while (itr != additional.end())
		{
			PeerSource* ps = *itr;
			ps->aboutToBeDestroyed();
			itr++;
		}
		qDeleteAll(additional);
		additional.clear();
	}
		
	
	void PeerSourceManager::addPeerSource(PeerSource* ps)
	{
		additional.append(ps);
		connect(ps,SIGNAL(peersReady( PeerSource* )),
				pman,SLOT(peerSourceReady( PeerSource* )));
	}
	
	void PeerSourceManager::removePeerSource(PeerSource* ps)
	{
		disconnect(ps,SIGNAL(peersReady( PeerSource* )),
				pman,SLOT(peerSourceReady( PeerSource* )));
		additional.removeAll(ps);
	}

	void PeerSourceManager::start()
	{
		if (started)
			return;
		
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->start();
			i++;
		}
		
		TrackerManager::start();
	}
		
	void PeerSourceManager::stop(WaitJob* wjob)
	{
		if (!started)
			return;
		
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->stop();
			i++;
		}
		
		TrackerManager::stop(wjob);
	}
		
	void PeerSourceManager::completed()
	{
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->completed();
			i++;
		}
		
		TrackerManager::completed();
	}

	
		
	void PeerSourceManager::manualUpdate()
	{
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->manualUpdate();
			i++;
		}
		
		TrackerManager::manualUpdate();
	}
	
	void PeerSourceManager::addDHT()
	{
		if(m_dht)
		{
			removePeerSource(m_dht);
			delete m_dht;
		}
		
		m_dht = new dht::DHTPeerSource(Globals::instance().getDHT(),tor->getInfoHash(),tor->getStats().torrent_name);
		for (Uint32 i = 0;i < tor->getNumDHTNodes();i++)
			m_dht->addDHTNode(tor->getDHTNode(i));
		
		// add the DHT source
		addPeerSource(m_dht);
	}

	void PeerSourceManager::removeDHT()
	{
		if (!m_dht)
			return;
		
		removePeerSource(m_dht);
		delete m_dht;
		m_dht = 0;
	}
	
	bool PeerSourceManager::dhtStarted()
	{
		return m_dht != 0;
	}
	
	
}

#include "peersourcemanager.moc"
