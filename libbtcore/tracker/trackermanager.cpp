/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
#include <QTextStream>
#include <QFile>
#include <klocale.h>
#include <util/log.h>
#include <tracker/tracker.h>
#include <tracker/udptracker.h>
#include <tracker/httptracker.h>
#include <torrent/torrentcontrol.h>
#include <torrent/torrent.h>
#include <peer/peermanager.h>
#include "trackermanager.h"

namespace bt
{
	const Uint32 INITIAL_WAIT_TIME = 30; 
	const Uint32 LONGER_WAIT_TIME = 300; 
	const Uint32 FINAL_WAIT_TIME = 1800;
	
	TrackerManager::TrackerManager(bt::TorrentControl* tor,PeerManager* pman) 
		: tor(tor),pman(pman),curr(0),started(false),pending(false)
	{
		trackers.setAutoDelete(true);
		no_save_custom_trackers = false;
		connect(&timer,SIGNAL(timeout()),this,SLOT(updateCurrentManually()));
		timer.setSingleShot(true);
		
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
		
		failures = 0;
		switchTracker(selectTracker());
	}

	TrackerManager::~TrackerManager() 
	{
		saveCustomURLs();
		saveTrackerStatus();
	}
	
	TrackerInterface* TrackerManager::getCurrentTracker() const 
	{
		return curr;
	}
	
	void TrackerManager::setCurrentTracker(bt::TrackerInterface* t) 
	{
		Tracker* trk = (Tracker*)t;
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
	
	void TrackerManager::setCurrentTracker(const KUrl& url) 
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

	
	QList<TrackerInterface*> TrackerManager::getTrackers() 
	{
		QList<TrackerInterface*> ret;
		for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
		{
			ret.append(i->second);
		}
		
		return ret;
	}
	
	TrackerInterface* TrackerManager::addTracker(const KUrl& url, bool custom, int tier) 
	{
		if (trackers.contains(url))
			return 0;
		
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
		
		return trk;
	}
	
	bool TrackerManager::removeTracker(bt::TrackerInterface* t) 
	{
		return removeTracker(t->trackerURL());
	}
	
	bool TrackerManager::removeTracker(const KUrl& url) 
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

	
	void TrackerManager::restoreDefault() 
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
	
	void TrackerManager::addTracker(Tracker* trk)
	{
		trackers.insert(trk->trackerURL(),trk);
		connect(trk,SIGNAL(peersReady( PeerSource* )),
				 pman,SLOT(peerSourceReady( PeerSource* )));
		connect(trk,SIGNAL(scrapeDone()),tor,SLOT(trackerScrapeDone()));
	}
	
	void TrackerManager::start() 
	{
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
	
	void TrackerManager::stop(bt::WaitJob* wjob) 
	{
		if (curr)
			curr->stop(wjob);
	}

	void TrackerManager::completed() 
	{
		if (curr)
			curr->completed();
	}

	void TrackerManager::scrape()
	{
		if (curr)
			curr->scrape();
	}
	
	void TrackerManager::manualUpdate() 
	{
		if (curr)
		{
			timer.stop();
			curr->manualUpdate();
		}
	}
	
	void TrackerManager::saveCustomURLs()
	{
		QString trackers_file = tor->getTorDir() + "trackers"; 
		QFile file(trackers_file);
		if(!file.open(QIODevice::WriteOnly))
			return;
		
		QTextStream stream(&file);
		for (KUrl::List::iterator i = custom_trackers.begin();i != custom_trackers.end();i++)
			stream << (*i).prettyUrl() << ::endl;
	}
	
	void TrackerManager::loadCustomURLs()
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
	
	void TrackerManager::saveTrackerStatus()
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
	
	void TrackerManager::loadTrackerStatus()
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
	
	Tracker* TrackerManager::selectTracker()
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
	
	void TrackerManager::onTrackerError(const QString & err)
	{
		failures++;
		pending = false;
		
		if (!started)
			return;
		
		// select an other tracker
		Tracker* trk = selectTracker();
		
		if (!trk || trk == curr)
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
	
	void TrackerManager::onTrackerOK()
	{
		failures = 0;
		if (started)
		{
			timer.start(curr->getInterval() * 1000);
			curr->scrape();
		}
		pending = false;
		request_time = QDateTime::currentDateTime();
	}
	
	void TrackerManager::onTrackerRequestPending()
	{
		pending = true;
	}
	
	void TrackerManager::updateCurrentManually()
	{
		if (!curr)
			return;
		
		if (!curr->isStarted())
			tor->resetTrackerStats();
		
		curr->manualUpdate();
	}
	
	void TrackerManager::switchTracker(Tracker* trk)
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
			connect(curr,SIGNAL(requestFailed( const QString& )),this,SLOT(onTrackerError( const QString& )));
			connect(curr,SIGNAL(requestOK()),this,SLOT(onTrackerOK()));
			connect(curr,SIGNAL(requestPending()),this,SLOT(onTrackerRequestPending()));
		}
	}
	
	Uint32 TrackerManager::getNumSeeders() const 
	{
		if (tor->getStats().priv_torrent)
			return curr ? curr->getNumSeeders() : 0;
		
		Uint32 r = 0;
		for (PtrMap<KUrl,Tracker>::const_iterator i = trackers.begin();i != trackers.end();i++)
			r += i->second->getNumSeeders();
		
		return r;
	}
	
	Uint32 TrackerManager::getNumLeechers() const 
	{
		if (tor->getStats().priv_torrent)
			return curr ? curr->getNumLeechers() : 0;
		
		Uint32 r = 0;
		for (PtrMap<KUrl,Tracker>::const_iterator i = trackers.begin();i != trackers.end();i++)
			r += i->second->getNumLeechers();
		
		return r;
	}


	/*
	
	*/
	
	void TrackerManager::setTrackerEnabled(const KUrl & url,bool enabled)
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

}