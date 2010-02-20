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

	
	TrackerManager::TrackerManager(bt::TorrentControl* tor,PeerManager* pman) 
		: tor(tor),pman(pman),curr(0),started(false)
	{
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
		
		if (tor->getStats().priv_torrent)
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
	
	bool TrackerManager::noTrackersReachable() const
	{
		if (tor->getStats().priv_torrent)
		{
			return curr ? curr->trackerStatus() == TRACKER_ERROR : false;
		}
		else
		{
			int enabled = 0;
			
			// If all trackers have an ERROR status, and there is at least one
			// enabled, we must return true;
			for (PtrMap<KUrl,Tracker>::const_iterator i = trackers.begin();i != trackers.end();i++)
			{
				if (i->second->isEnabled())
				{
					if (i->second->trackerStatus() != TRACKER_ERROR)
						return false;
					enabled++;
				}
			}
			
			return enabled > 0;
		}
	}

	
	void TrackerManager::setCurrentTracker(bt::TrackerInterface* t) 
	{
		if (!tor->getStats().priv_torrent)
			return;
		
		Tracker* trk = (Tracker*)t;
		if (!trk)
			return;
		
		if (curr != trk)
		{
			if (curr)
				curr->stop();
			switchTracker(trk);
			trk->start();
		}
	}
	
	void TrackerManager::setCurrentTracker(const KUrl& url) 
	{
		Tracker* trk = trackers.find(url);
		if (trk)
			setCurrentTracker(trk);
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
			trk = new UDPTracker(url,this,tor->getTorrent().getPeerID(),tier);
		else
			trk = new HTTPTracker(url,this,tor->getTorrent().getPeerID(),tier);
		
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
		if (curr == trk && tor->getStats().priv_torrent)
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

	bool TrackerManager::canRemoveTracker(bt::TrackerInterface* t) 
	{
		return custom_trackers.contains(t->trackerURL());
	}

	
	void TrackerManager::restoreDefault() 
	{
		KUrl::List::iterator i = custom_trackers.begin();
		while (i != custom_trackers.end())
		{
			Tracker* t = trackers.find(*i);
			if (t)
			{
				if (t->isStarted())
					t->stop();
				
				if (curr == t && tor->getStats().priv_torrent)
				{
					curr = 0;
					trackers.erase(*i);
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
		if (tor->getStats().priv_torrent && curr == 0)
			switchTracker(selectTracker());
	}
	
	void TrackerManager::addTracker(Tracker* trk)
	{
		trackers.insert(trk->trackerURL(),trk);
		connect(trk,SIGNAL(peersReady( PeerSource* )),
				 pman,SLOT(peerSourceReady( PeerSource* )));
		connect(trk,SIGNAL(scrapeDone()),tor,SLOT(trackerScrapeDone()));
		connect(trk,SIGNAL(requestOK()),this,SLOT(onTrackerOK()));
		connect(trk,SIGNAL(requestFailed( const QString& )),this,SLOT(onTrackerError( const QString& )));
	}
	
	void TrackerManager::start() 
	{
		if (started)
			return;
		
		if (tor->getStats().priv_torrent)
		{
			if (!curr)
			{
				if (trackers.count() > 0)
				{
					switchTracker(selectTracker());
					if (curr)
						curr->start();
				}
			}
			else
			{
				curr->start();
			}
		}
		else
		{
			for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
			{
				if (i->second->isEnabled())
					i->second->start();
			}
		}
		
		started = true;
	}
	
	void TrackerManager::stop(bt::WaitJob* wjob) 
	{
		if (!started)
			return;
		
		started = false;
		if (tor->getStats().priv_torrent)
		{
			if (curr)
				curr->stop(wjob);
			
			for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
			{
				i->second->reset();
			}
		}
		else
		{
			for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
			{
				i->second->stop(wjob);
				i->second->reset();
			}
		}
	}

	void TrackerManager::completed() 
	{
		if (tor->getStats().priv_torrent)
		{
			if (curr)
				curr->completed();
		}
		else
		{
			for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
			{
				i->second->completed();
			}
		}
	}

	void TrackerManager::scrape()
	{
		for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
		{
			i->second->scrape();
		}
	}
	
	void TrackerManager::manualUpdate() 
	{
		if (tor->getStats().priv_torrent)
		{
			if (curr)
			{
				curr->manualUpdate();
			}
		}
		else
		{
			for (PtrMap<KUrl,Tracker>::iterator i = trackers.begin();i != trackers.end();i++)
			{
				if (i->second->isEnabled())
					i->second->manualUpdate();
			}
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
		Q_UNUSED(err);
		if (!started)
			return;
		
		if (!tor->getStats().priv_torrent)
		{
			Tracker* trk = (Tracker*)sender();
			trk->handleFailure();
		}
		else 
		{
			Tracker* trk = (Tracker*)sender();
			if (trk == curr)
			{
				// select an other tracker
				trk = selectTracker();
				if (trk == curr) // if we can't find another handle the failure
				{
					trk->handleFailure();
				}
				else
				{
					curr->stop();
					switchTracker(trk);
					if (curr->failureCount() > 0)
						curr->handleFailure();
					else
						curr->start();
				}
			}
			else
				trk->handleFailure();
		}
	}
	
	void TrackerManager::onTrackerOK()
	{
		Tracker* tracker = (Tracker*)sender();
		if (tracker->isStarted())
			tracker->scrape();
	}
	
	void TrackerManager::updateCurrentManually()
	{
		if (!curr)
			return;
		
		curr->manualUpdate();
	}
	
	void TrackerManager::switchTracker(Tracker* trk)
	{
		if (curr == trk)
			return;
		
		curr = trk;
		if (curr)
			Out(SYS_TRK|LOG_NOTICE) << "Switching to tracker " << trk->trackerURL() << endl;
	}
	
	Uint32 TrackerManager::getNumSeeders() const 
	{
		if (tor->getStats().priv_torrent)
		{
			return curr && curr->getNumSeeders() > 0 ? curr->getNumSeeders() : 0;
		}
		
		Uint32 r = 0;
		for (PtrMap<KUrl,Tracker>::const_iterator i = trackers.begin();i != trackers.end();i++)
		{
			int v = i->second->getNumSeeders();
			if (v > 0)
				r += v;
		}
		
		return r;
	}
	
	Uint32 TrackerManager::getNumLeechers() const 
	{
		if (tor->getStats().priv_torrent)
			return curr && curr->getNumLeechers() > 0 ? curr->getNumLeechers() : 0;
		
		Uint32 r = 0;
		for (PtrMap<KUrl,Tracker>::const_iterator i = trackers.begin();i != trackers.end();i++)
		{
			int v = i->second->getNumLeechers();
			if (v > 0)
				r += v;
		}
		
		return r;
	}
	
	void TrackerManager::setTrackerEnabled(const KUrl & url,bool enabled)
	{
		Tracker* trk = trackers.find(url);
		if (!trk)
			return;
		
		trk->setEnabled(enabled);
		if (!enabled) 
		{
			trk->stop();
			if (curr == trk) // if the current tracker is disabled, switch to another one
			{
				switchTracker(selectTracker());
				if (curr)
					curr->start();
			}
		}
		else
		{
			// start tracker if necessary
			if (!tor->getStats().priv_torrent && started)
				trk->start();
		}
		
		saveTrackerStatus();
	}

	Uint64 TrackerManager::bytesDownloaded() const
	{
		return tor->getStats().bytes_downloaded;
	}

	Uint64 TrackerManager::bytesUploaded() const
	{
		return tor->getStats().bytes_uploaded;
	}
	
	Uint64 TrackerManager::bytesLeft() const
	{
		return tor->getStats().bytes_left;
	}
	
	const bt::SHA1Hash& TrackerManager::infoHash() const
	{
		return tor->getInfoHash();
	}


}