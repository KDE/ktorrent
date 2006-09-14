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
#include <qfile.h>
#include <klocale.h>
#include <functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <kademlia/dhtbase.h>
#include <kademlia/dhttrackerbackend.h>
#include "tracker.h"
#include "udptracker.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "torrent.h"
#include "peermanager.h"
#include "peersourcemanager.h"

namespace bt
{

	PeerSourceManager::PeerSourceManager(TorrentControl* tor,PeerManager* pman) 
	: tor(tor),pman(pman),curr(0),started(false),pending(false)
	{
		failures = 0;
		trackers.setAutoDelete(true);
		no_save_custom_trackers = false;
		
		// add all standard trackers
		const KURL::List & tr = tor->getTorrent().getTrackerList();
		KURL::List::const_iterator i = tr.begin();
		while (i != tr.end())
		{
			addTracker(*i,false);
			i++;
		}
		
		loadCustomURLs();
		
		// add the DHT source
		addPeerSource(new dht::DHTTrackerBackend(Globals::instance().getDHT(),tor));
	}
	
	PeerSourceManager::~PeerSourceManager()
	{
		saveCustomURLs();
		additional.setAutoDelete(true);
	}
		
	void PeerSourceManager::addTracker(Tracker* trk)
	{
		trackers.insert(trk->trackerURL(),trk);
		connect(trk,SIGNAL(peersReady( kt::PeerSource* )),
				 pman,SLOT(peerSourceReady( kt::PeerSource* )));
	}
		
	void PeerSourceManager::addPeerSource(kt::PeerSource* ps)
	{
		additional.append(ps);
		connect(ps,SIGNAL(peersReady( kt::PeerSource* )),
						 pman,SLOT(peerSourceReady( kt::PeerSource* )));
	}
	
	void PeerSourceManager::removePeerSource(kt::PeerSource* ps)
	{
		disconnect(ps,SIGNAL(peersReady( kt::PeerSource* )),
				pman,SLOT(peerSourceReady( kt::PeerSource* )));
		additional.remove(ps);
	}

	void PeerSourceManager::start()
	{
		started = true;
		QPtrList<kt::PeerSource>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->start();
			i++;
		}
		
		if (!curr)
		{
			if (trackers.count() > 0)
			{
				switchTracker(trackers.begin()->second);
				tor->resetTrackerStats();
				curr->start();
			}
		}
		else
		{
			curr->start();
		}
	}
		
	void PeerSourceManager::stop()
	{
		started = false;
		QPtrList<kt::PeerSource>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->stop();
			i++;
		}
		
		if (curr)
			curr->stop();
		
		statusChanged(i18n("Stopped"));
	}
		
	void PeerSourceManager::completed()
	{
		QPtrList<kt::PeerSource>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->completed();
			i++;
		}
		
		if (curr)
			curr->completed();
	}
		
	void PeerSourceManager::manualUpdate()
	{
		QPtrList<kt::PeerSource>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->manualUpdate();
			i++;
		}
		
		if (curr)
		{
			timer.stop();
			curr->manualUpdate();
		}
	}
		
	
	
	KURL PeerSourceManager::getTrackerURL() const
	{
		if (curr)
			return curr->trackerURL();
		else
			return KURL();
	}
	
	KURL::List PeerSourceManager::getTrackerURLs()
	{
		KURL::List urls = tor->getTorrent().getTrackerList();
		urls += custom_trackers; 
		return urls; 
	}
	
	void PeerSourceManager::addTracker(KURL url, bool custom)
	{
		Tracker* trk = 0;
		if (url.protocol() == "udp")
			trk = new UDPTracker(url,tor,tor->getTorrent().getPeerID());
		else
			trk = new HTTPTracker(url,tor,tor->getTorrent().getPeerID());
		
		addTracker(trk);
		if (custom)
		{
			custom_trackers.append(url);
			if (!no_save_custom_trackers)
				saveCustomURLs();
		}
	}
	
	bool PeerSourceManager::removeTracker(KURL url)
	{
		if (!custom_trackers.contains(url))
			return false;
		
		custom_trackers.remove(url);
		Tracker* trk = trackers.find(url);
		if (curr == trk)
		{
			// do a timed delete on the tracker, so the stop signal
			// has plenty of time to reach it
			trk->stop();
			trk->timedDelete(10 * 1000);
			trackers.setAutoDelete(false);
			trackers.erase(url);
			trackers.setAutoDelete(true);
			
			if (trackers.count() > 0)
			{
				switchTracker(trackers.begin()->second);
				tor->resetTrackerStats();
				curr->start();
			}
		}
		else
		{
			// just delete if not the current one
			trackers.erase(url);
		}
		saveCustomURLs();
		return true;
	}
	
	void PeerSourceManager::setTracker(KURL url)
	{
		Tracker* trk = trackers.find(url);
		if (!trk)
			return;
		
		if (curr != trk)
		{
			curr->stop();
			switchTracker(trk);
			tor->resetTrackerStats();
			trk->start();
		}
	}
	
	void PeerSourceManager::restoreDefault()
	{
		KURL::List::iterator i = custom_trackers.begin();
		while (i != custom_trackers.end())
		{
			Tracker* t = trackers.find(*i);
			if (t)
			{
				if (curr == t)
				{
					if (t->isStarted())
						t->stop();
					
					curr = 0;
					trackers.erase(*i);
					if (trackers.count() > 0)
					{
						switchTracker(trackers.begin()->second);
						if (started)
						{
							tor->resetTrackerStats();
							curr->start();
						}
					}
				}
				else
				{
					trackers.erase(*i);
				}
			}
			i++;
		}
		
		custom_trackers.clear();
		saveCustomURLs();
	}
	
	void PeerSourceManager::saveCustomURLs()
	{
		QString trackers_file = tor->getTorDir() + "trackers"; 
		QFile file(trackers_file);
		if(!file.open(IO_WriteOnly))
			return;
		
		QTextStream stream(&file);
		for (KURL::List::iterator i = custom_trackers.begin();i != custom_trackers.end();i++)
			stream << (*i).prettyURL() << ::endl;
	}
	
	void PeerSourceManager::loadCustomURLs()
	{
		QString trackers_file = tor->getTorDir() + "trackers";
		QFile file(trackers_file);
		if(!file.open(IO_ReadOnly))
			return;
		
		no_save_custom_trackers = true;
		QTextStream stream(&file);
		while (!stream.atEnd())
		{
			KURL url = stream.readLine();
			addTracker(url,true);
		}
		no_save_custom_trackers = false;
	}
	
	void PeerSourceManager::onTrackerError(const QString & err)
	{
		failures++;
		pending = false;
		if (started)
			statusChanged(err);
		
		if (!started)
			return;
		
		// select an other tracker
		Tracker* trk = 0;
		PtrMap<KURL,Tracker>::iterator i = trackers.begin();
		while (i != trackers.end())
		{
			if (i->second != curr)
			{
				if (!trk)
					trk = i->second;
				else if (i->second->failureCount() < trk->failureCount())
					trk = i->second;
			}
				
			i++;
		}
		
		if (!trk)
		{
			if (failures >= 3)
			{
				// we failed to contact the only tracker 3 times in a row, so try again in 
				// a minute or 5, no need for hammering every 30 seconds
				curr->setInterval(5 * 60);
				timer.start(5 * 60 * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else
			{
				// lets not hammer and wait 30 seconds
				curr->setInterval(30);
				timer.start(30 * 1000);
				request_time = QDateTime::currentDateTime();
			}
		}
		else
		{
			curr->stop();
			// switch to another one
			switchTracker(trk);
			if (trk->failureCount() == 0)
			{
				tor->resetTrackerStats();
				curr->start();
			}
			else if (trk->failureCount() >= 3)
			{
				// we tried everybody 3 times and it didn't work
				// wait 5 minutes and try again
				curr->setInterval(5 * 60);
				timer.start(5 * 60 * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else
			{
				// wait 30 seconds and try again
				curr->setInterval(30);
				timer.start(30 * 1000);
				request_time = QDateTime::currentDateTime();
			}
		}
	}
		
	void PeerSourceManager::onTrackerOK()
	{
		failures = 0;
		if (started)
		{
			timer.start(curr->getInterval() * 1000,true);
			curr->scrape();
		}
		pending = false;
		if (started)
			statusChanged(i18n("OK"));
		request_time = QDateTime::currentDateTime();
	}
		
	void PeerSourceManager::onTrackerRequestPending()
	{
		if (started)
			statusChanged(i18n("Announcing"));
		pending = true;
	}
	
	void PeerSourceManager::switchTracker(Tracker* trk)
	{
		if (curr == trk)
			return;
		
		Out(SYS_TRK|LOG_NOTICE) << "Switching to tracker " << trk->trackerURL() << endl;
		if (curr)
		{
			disconnect(curr,SIGNAL(requestFailed( const QString& )),
					   this,SLOT(onTrackerError( const QString& )));
			disconnect(curr,SIGNAL(requestOK()),this,SLOT(onTrackerOK()));
			disconnect(curr,SIGNAL(requestPending()),this,SLOT(onTrackerRequestPending()));
			disconnect(&timer,SIGNAL(timeout()),curr,SLOT(manualUpdate()));
			curr = 0;
		}
		
		curr = trk;
		QObject::connect(curr,SIGNAL(requestFailed( const QString& )),
				this,SLOT(onTrackerError( const QString& )));
		
		QObject::connect(curr,SIGNAL(requestOK()),
				this,SLOT(onTrackerOK()));
		
		QObject::connect(curr,SIGNAL(requestPending()),
				this,SLOT(onTrackerRequestPending()));
		
		QObject::connect(&timer,SIGNAL(timeout()),
				 curr,SLOT(manualUpdate()));
	}
	
	Uint32 PeerSourceManager::getTimeToNextUpdate() const
	{
		if (pending || !started || !curr)
			return 0;
		
		return curr->getInterval() - request_time.secsTo(QDateTime::currentDateTime());
	}
	
	Uint32 PeerSourceManager::getNumSeeders() const
	{
		return curr ? curr->getNumSeeders() : 0;
	}
		
	
	Uint32 PeerSourceManager::getNumLeechers() const
	{
		return curr ? curr->getNumLeechers() : 0;
	}
}

#include "peersourcemanager.moc"
