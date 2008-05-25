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
	static QString custom_ip;
	static QString custom_ip_resolved;
	
	Tracker::Tracker(const KURL & url,kt::TorrentInterface* tor,const PeerID & id,int tier) 
	: url(url),tier(tier),peer_id(id),tor(tor)
	{
		// default 5 minute interval
		interval = 5 * 60 * 1000;
		seeders = leechers = 0;
		srand(time(0));
		key = rand();
		started = false;
	}
	
	Tracker::~Tracker()
	{
	}
	
	void Tracker::setCustomIP(const QString & ip)
	{
		if (custom_ip == ip)
			return;
		
		Out(SYS_TRK|LOG_NOTICE) << "Setting custom ip to " << ip << endl;
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
			Out(SYS_TRK|LOG_NOTICE) << "custom_ip_resolved = " << custom_ip_resolved << endl;
		}
	}
	
	QString Tracker::getCustomIP()
	{
		return custom_ip_resolved;
	}
	
	void Tracker::timedDelete(int ms)
	{
		QTimer::singleShot(ms,this,SLOT(deleteLater()));
		connect(this,SIGNAL(stopDone()),this,SLOT(deleteLater()));
	}
	
}

#include "tracker.moc"
