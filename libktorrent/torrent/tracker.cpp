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
#include <stdlib.h>
#include <time.h>
#include <kurl.h>
#include <kresolver.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/torrentinterface.h>
#include <kademlia/dhtbase.h>
#include <kademlia/dhttrackerbackend.h>
#include "server.h"
#include "tracker.h"
#include "udptracker.h"
#include "httptracker.h"

using namespace KNetwork;

namespace bt
{
	QString Tracker::custom_ip;
	QString Tracker::custom_ip_resolved;
	
	TrackerBackend::TrackerBackend(Tracker* trk) : frontend(trk)
	{}
	
	TrackerBackend::~TrackerBackend()
	{}

	Tracker::Tracker(kt::TorrentInterface* tor,
					 const SHA1Hash & ih,const PeerID & id) : tor(tor)
	{
		info_hash = ih;
		peer_id = id;
		interval = 120;
		seeders = leechers = 0;
		num_failed_attempts = 0;
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		connect(&error_update_timer,SIGNAL(timeout()),this,SLOT(onErrorTimeout()));
		connect(&dht_update_timer,SIGNAL(timeout()),this,SLOT(onDHTUpdate()));
		error_mode = false;
		
		srand(time(0));
		key = rand();
		udp = http = curr = dht_ba = 0;
		if (!tor->getStats().priv_torrent && Globals::instance().getDHT().isRunning())
			dht_ba = new dht::DHTTrackerBackend(this,Globals::instance().getDHT());
	}


	Tracker::~Tracker()
	{
		delete dht_ba;
		delete udp;
		delete http;
	}
	
	void Tracker::doRequest(const KURL & url)
	{
		if (!url.isValid())
			return;
		
		if (url.protocol() == "udp" || url.prettyURL().startsWith("udp"))
		{
			if (!udp)
				udp = new UDPTracker(this);
			
			udp->doRequest(url);
			curr = udp;
		}
		else
		{
			if (!http)
				http = new HTTPTracker(this);
			
			http->doRequest(url);
			curr = http;
		}
		last_url = url;
	}

	void Tracker::updateData(PeerManager* pman)
	{
		if (curr)
			curr->updateData(pman);
		
		if (dht_ba)
			dht_ba->updateData(pman);
	}

	void Tracker::start()
	{
		event = "started";
		doRequest(tor->getTrackerURL(true));
		update_timer.start(interval*1000);
		time_of_last_update = GetCurrentTime();
		// start the DHT after one minute, so we can get some peers first
		if (dht_ba)
		{
			// start after 15 seconds
			dht_update_timer.start(15*1000,true);
		}
	}
		
	void Tracker::setInterval(Uint32 secs)
	{
		if (secs == 0)
			secs = 120;
		
		if (interval != secs)
		{
			update_timer.changeInterval(1000*secs);
			time_of_last_update = GetCurrentTime();
		}
		interval = secs;
	}
		
	void Tracker::stop()
	{
		event = "stopped";
		doRequest(/*tor->getTrackerURL(true)*/last_url);
		update_timer.stop();
		dht_update_timer.stop();
	}

	void Tracker::onTimeout()
	{
		if (!error_mode)
		{
			event = QString::null;
			doRequest(tor->getTrackerURL(true));
			time_of_last_update = GetCurrentTime();
		}
	}

	void Tracker::onErrorTimeout()
	{
		doRequest(tor->getTrackerURL(false));
		time_of_last_update = GetCurrentTime();
	}
	
	void Tracker::onDHTUpdate()
	{
		if (dht_ba && event != "stopped")
		{
			Uint16 port = Globals::instance().getServer().getPortInUse();
			if (dht_ba->doRequest(QString("http://localhost:%1/announce").arg(port)))
			{
				// do the next update in 15 minutes
				dht_update_timer.start(15*60*1000,true);
			}
			else
			{
				// not succes full try again in 10 seconds
				dht_update_timer.start(10*1000,true);
			}
		}
	}

	void Tracker::updateOK()
	{
		error_mode = false;
		num_failed_attempts = 0;
		error_update_timer.stop();
	}

	void Tracker::handleError()
	{
		if (event != "stopped")
		{
			error_mode = true;
			num_failed_attempts++;
			// first try 5 times in a row
			// after 5 attempts switch to a 30 second delay
			if (num_failed_attempts < 5)
			{
				doRequest(tor->getTrackerURL(false));
				time_of_last_update = GetCurrentTime();
			}
			else
				error_update_timer.start(30*1000,true);
		}
	}
	
	

	void Tracker::manualUpdate()
	{
		event = QString::null;
		doRequest(tor->getTrackerURL(true));
		time_of_last_update = GetCurrentTime();
		
		// start DHT backend if we can
		if (!dht_ba && !tor->getStats().priv_torrent && Globals::instance().getDHT().isRunning())
			dht_ba = new dht::DHTTrackerBackend(this,Globals::instance().getDHT());
		
		if (dht_ba)
		{
			dht_update_timer.stop();
			Uint16 port = Globals::instance().getServer().getPortInUse();
			dht_ba->doRequest(QString("http://localhost:%1/announce").arg(port));
			dht_update_timer.start(15*60*1000,true);
		}
	}

	void Tracker::completed()
	{
		event = "completed";
		doRequest(tor->getTrackerURL(true));
		time_of_last_update = GetCurrentTime();
	}

	Uint32 Tracker::getTimeToNextUpdate() const
	{
		Uint32 s = (GetCurrentTime() - time_of_last_update) / 1000;
		if (error_mode)
		{
			if (s > 30)
				return 0;
			else
				return 30 - s;
		}
		else
		{
			if (s > interval)
				return 0;
			else
				return interval - s;
		}
	}
	
	void Tracker::setCustomIP(const QString & ip)
	{
		if (custom_ip == ip)
			return;
		
		Out() << "Setting custom ip to " << ip << endl;
		custom_ip = ip;
		custom_ip_resolved = QString::null;
		if (ip.isNull())
			return;
		
		KResolverResults res = KResolver::resolve(ip,QString::null);
		if (res.error() || res.empty())
		{
			custom_ip = custom_ip_resolved = QString::null;
		}
		else
		{
			custom_ip_resolved = res.first().address().nodeName();
			Out() << "custom_ip_resolved = " << custom_ip_resolved << endl;
		}
	}
}

#include "tracker.moc"
