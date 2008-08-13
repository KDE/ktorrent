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
#include <dht/dhttrackerbackend.h>
#include <tracker/tracker.h>
#include <tracker/udptracker.h>
#include <tracker/httptracker.h>
#include "torrentcontrol.h"
#include "torrent.h"
#include <peer/peermanager.h>

namespace bt
{
	const Uint32 INITIAL_WAIT_TIME = 30; 
	const Uint32 LONGER_WAIT_TIME = 300; 
	const Uint32 FINAL_WAIT_TIME = 1800;

	PeerSourceManager::PeerSourceManager(TorrentControl* tor,PeerManager* pman) 
	: tor(tor),pman(pman),curr(0),m_dht(0),started(false),pending(false)
	{
		failures = 0;
		trackers.setAutoDelete(true);
		no_save_custom_trackers = false;
		
		const TrackerTier* t = tor->getTorrent().getTrackerList();
		int tier = 1;
		while (t)
		{
			// add all standard trackers
			const KUrl::List & tr = t->urls;
			KUrl::List::const_iterator i = tr.begin();
			while (i != tr.end())
			{
				addTracker(*i,false,tier);
				i++;
			}
			
			tier++;
			t = t->next;
		}
		
		//load custom trackers
		loadCustomURLs();
		// Load status of each tracker
		loadTrackerStatus();
		
		connect(&timer,SIGNAL(timeout()),this,SLOT(updateCurrentManually()));
		timer.setSingleShot(true);
		switchTracker(selectTracker());
	}
	
	PeerSourceManager::~PeerSourceManager()
	{
		saveCustomURLs();
		saveTrackerStatus();
		
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
		
	void PeerSourceManager::addTracker(Tracker* trk)
	{
		trackers.insert(trk->trackerURL(),trk);
		connect(trk,SIGNAL(peersReady( PeerSource* )),
				 pman,SLOT(peerSourceReady( PeerSource* )));
		connect(trk,SIGNAL(scrapeDone()),tor,SLOT(trackerScrapeDone()));
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
		
		started = true;
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->start();
			i++;
		}
		
		if (!curr)
		{
			if (trackers.count() > 0)
			{
				switchTracker(selectTracker());
				tor->resetTrackerStats();
				if (curr)
					curr->start();
			}
		}
		else
		{
			tor->resetTrackerStats();
			curr->start();
		}
	}
		
	void PeerSourceManager::stop(WaitJob* wjob)
	{
		if (!started)
			return;
		
		started = false;
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->stop();
			i++;
		}
		
		if (curr)
			curr->stop(wjob);
		
		timer.stop();
		statusChanged(i18n("Stopped"));
	}
		
	void PeerSourceManager::completed()
	{
		QList<PeerSource*>::iterator i = additional.begin();
		while (i != additional.end())
		{
			(*i)->completed();
			i++;
		}
		
		if (curr)
			curr->completed();
	}

	void PeerSourceManager::scrape()
	{
		if (curr)
			curr->scrape();
	}
		
	void PeerSourceManager::manualUpdate()
	{
		QList<PeerSource*>::iterator i = additional.begin();
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
		
	
	
	KUrl PeerSourceManager::getTrackerURL() const
	{
		if (curr)
			return curr->trackerURL();
		else
			return KUrl();
	}
	
	KUrl::List PeerSourceManager::getTrackerURLs()
	{
		KUrl::List urls;
		const TrackerTier* t = tor->getTorrent().getTrackerList();
		while (t)
		{
			foreach (const KUrl & u,t->urls)
				if (!urls.contains(u))
					urls += u;
			t = t->next;
		}
		
		urls += custom_trackers; 
		return urls; 
	}
	
	void PeerSourceManager::addTracker(const KUrl &url, bool custom,int tier)
	{
		if (trackers.contains(url))
			return;
		
		Tracker* trk = 0;
		if (url.protocol() == "udp")
			trk = new UDPTracker(url,tor,tor->getTorrent().getPeerID(),tier);
		else
			trk = new HTTPTracker(url,tor,tor->getTorrent().getPeerID(),tier);
		
		addTracker(trk);
		if (custom)
		{
			custom_trackers.append(url);
			if (!no_save_custom_trackers)
			{
				saveCustomURLs();
				saveTrackerStatus();
			}
		}
	}
	
	bool PeerSourceManager::removeTracker(const KUrl &url)
	{
		if (!custom_trackers.contains(url))
			return false;
		
		custom_trackers.removeAll(url);
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
				switchTracker(selectTracker());
				tor->resetTrackerStats();
				if (curr)
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
	
	void PeerSourceManager::setTracker(const KUrl &url)
	{
		Tracker* trk = trackers.find(url);
		if (!trk)
			return;
		
		if (curr != trk)
		{
			if (curr)
				curr->stop();
			switchTracker(trk);
			tor->resetTrackerStats();
			trk->start();
		}
	}
	
	void PeerSourceManager::restoreDefault()
	{
		KUrl::List::iterator i = custom_trackers.begin();
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
	
	void PeerSourceManager::setTrackerEnabled(const KUrl & url,bool enabled)
	{
		Tracker* trk = trackers.find(url);
		if (!trk)
			return;
		
		trk->setEnabled(enabled);
		if (!enabled && curr == trk) // if the current tracker is disabled, switch to another one
		{
			curr->stop();
			switchTracker(selectTracker());
			tor->resetTrackerStats();
			if (curr)
				curr->start();
		}
		saveTrackerStatus();
	}
	
	bool PeerSourceManager::isTrackerEnabled(const KUrl & url) const
	{
		const Tracker* trk = trackers.find(url);
		if (!trk)
			return false;
		else
			return trk->isEnabled();
	}
	
	void PeerSourceManager::saveCustomURLs()
	{
		QString trackers_file = tor->getTorDir() + "trackers"; 
		QFile file(trackers_file);
		if(!file.open(QIODevice::WriteOnly))
			return;
		
		QTextStream stream(&file);
		for (KUrl::List::iterator i = custom_trackers.begin();i != custom_trackers.end();i++)
			stream << (*i).prettyUrl() << ::endl;
	}
	
	void PeerSourceManager::loadCustomURLs()
	{
		QString trackers_file = tor->getTorDir() + "trackers";
		QFile file(trackers_file);
		if(!file.open( QIODevice::ReadOnly))
			return;
		
		no_save_custom_trackers = true;
		QTextStream stream(&file);
		while (!stream.atEnd())
		{
			KUrl url = stream.readLine();
			addTracker(url,true);
		}
		no_save_custom_trackers = false;
	}
	
	void PeerSourceManager::saveTrackerStatus()
	{
		QString status_file = tor->getTorDir() + "tracker_status"; 
		QFile file(status_file);
		if(!file.open(QIODevice::WriteOnly))
			return;
		
		QTextStream stream(&file);
		PtrMap<KUrl,Tracker>::iterator i = trackers.begin();
		while (i != trackers.end())
		{
			KUrl url = i->first;
			Tracker* trk = i->second;
			
			stream << (trk->isEnabled() ? "1:" : "0:") << url.prettyUrl() << ::endl;
			i++;
		}
	}
	
	void PeerSourceManager::loadTrackerStatus()
	{
		QString status_file = tor->getTorDir() + "tracker_status"; 
		QFile file(status_file);
		if(!file.open( QIODevice::ReadOnly))
			return;
		
		QTextStream stream(&file);
		while (!stream.atEnd())
		{
			QString line = stream.readLine();
			if (line.size() < 2)
				continue;
			
			KUrl u = line.mid(2); // url starts at the second char
			if (line[0] == '0')
			{
				Tracker* trk = trackers.find(u);
				if (trk)
					trk->setEnabled(false);
			}
		}
	}
	
	Tracker* PeerSourceManager::selectTracker()
	{
		Tracker* n = 0;
		PtrMap<KUrl,Tracker>::iterator i = trackers.begin();
		while (i != trackers.end())
		{
			Tracker* t = i->second;
			if (t->isEnabled())
			{
				if (!n)
					n = t;
				else if (t->failureCount() < n->failureCount())
					n = t;
				else if (t->failureCount() == n->failureCount())
					n = t->getTier() < n->getTier() ? t : n;
			}
			i++;
		}
		
		if (n)
		{
			Out(SYS_TRK|LOG_DEBUG) << "Selected tracker " << n->trackerURL().prettyUrl() 
					<< " (tier = " << n->getTier() << ")" << endl;
		}
		
		return n;
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
		Tracker* trk = selectTracker();
		
		if (!trk)
		{
			if (curr->failureCount() > 5)
			{
				// we failed to contact the only tracker 5 times in a row, so try again in 
				// 30 minutes
				curr->setInterval(FINAL_WAIT_TIME);
				timer.start(FINAL_WAIT_TIME * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else if (curr->failureCount() > 2)
			{
				// we failed to contact the only tracker 3 times in a row, so try again in 
				// a minute or 5, no need for hammering every 30 seconds
				curr->setInterval(LONGER_WAIT_TIME);
				timer.start(LONGER_WAIT_TIME * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else
			{
				// lets not hammer and wait 30 seconds
				curr->setInterval(INITIAL_WAIT_TIME);
				timer.start(INITIAL_WAIT_TIME * 1000);
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
			else if (trk->failureCount() > 5)
			{
				curr->setInterval(FINAL_WAIT_TIME);
				timer.start(FINAL_WAIT_TIME * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else if (trk->failureCount() > 2)
			{
				// we tried everybody 3 times and it didn't work
				// wait 5 minutes and try again
				curr->setInterval(LONGER_WAIT_TIME);
				timer.start(LONGER_WAIT_TIME * 1000);
				request_time = QDateTime::currentDateTime();
			}
			else
			{
				// wait 30 seconds and try again
				curr->setInterval(INITIAL_WAIT_TIME);
				timer.start(INITIAL_WAIT_TIME * 1000);
				request_time = QDateTime::currentDateTime();
			}
		}
	}
		
	void PeerSourceManager::onTrackerOK()
	{
		failures = 0;
		if (started)
		{
			timer.start(curr->getInterval() * 1000);
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
	
	void PeerSourceManager::updateCurrentManually()
	{
		if (!curr)
			return;
		
		if (!curr->isStarted())
			tor->resetTrackerStats();
		
		curr->manualUpdate();
	}
	
	void PeerSourceManager::switchTracker(Tracker* trk)
	{
		if (curr == trk)
			return;
	
		if (curr)
		{
			disconnect(curr,SIGNAL(requestFailed( const QString& )),
					   this,SLOT(onTrackerError( const QString& )));
			disconnect(curr,SIGNAL(requestOK()),this,SLOT(onTrackerOK()));
			disconnect(curr,SIGNAL(requestPending()),this,SLOT(onTrackerRequestPending()));
			curr = 0;
		}
		
		curr = trk;
		if (curr)
		{
			Out(SYS_TRK|LOG_NOTICE) << "Switching to tracker " << trk->trackerURL() << endl;
			QObject::connect(curr,SIGNAL(requestFailed( const QString& )),
					this,SLOT(onTrackerError( const QString& )));
			
			QObject::connect(curr,SIGNAL(requestOK()),
					this,SLOT(onTrackerOK()));
			
			QObject::connect(curr,SIGNAL(requestPending()),
					this,SLOT(onTrackerRequestPending()));
		}
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
	
	Uint32 PeerSourceManager::getTotalTimesDownloaded() const
	{
		return curr ? curr->getTotalTimesDownloaded() : 0;
	}
	
	void PeerSourceManager::addDHT()
	{
		if(m_dht)
		{
			removePeerSource(m_dht);
			delete m_dht;
		}
		
		m_dht = new dht::DHTTrackerBackend(Globals::instance().getDHT(),tor);
		
		// add the DHT source
		addPeerSource(m_dht);
	}

	void PeerSourceManager::removeDHT()
	{
		if(m_dht == 0)
		{
			removePeerSource(m_dht);
			return;
		}
		
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
