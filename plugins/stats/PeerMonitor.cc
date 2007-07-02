/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "PeerMonitor.h"

namespace kt {

PeerMonitor::PeerMonitor(kt::TorrentInterface * pTi, std::map<bt::SHA1Hash, PeerMonitor *> * pM) : kt::MonitorInterface(), QObject(), pmTorIface(pTi), pmPeerMMgr(pM)
{

}

PeerMonitor::~PeerMonitor()
{
}

void PeerMonitor::peerAdded (kt::PeerInterface *peer)
{
	QMutexLocker lock(&mtx);
	
	mPeers.push_back( peer );
}

void PeerMonitor::peerRemoved (kt::PeerInterface *peer)
{

	QMutexLocker lock(&mtx);

	data_t::iterator it = std::find(mPeers.begin(), mPeers.end(), peer);
	
	if(it != mPeers.end())
	{
		mPeers.erase(it);
	//	*it = 0;
	}

}

void PeerMonitor::downloadStarted (kt::ChunkDownloadInterface *) 
{ 
	
}

void PeerMonitor::downloadRemoved (kt::ChunkDownloadInterface *) 
{ 
	
}

void PeerMonitor::stopped () 
{
	QMutexLocker lock(&mtx);
	
	std::fill(mPeers.begin(), mPeers.end(), static_cast<PeerInterface*>( 0 ) );
//	mPeers.clear();
}

void PeerMonitor::destroyed () 
{ 
	if(pmPeerMMgr -> find(pmTorIface -> getInfoHash()) != pmPeerMMgr -> end() )
	{
		pmTorIface -> setMonitor(0);
		pmPeerMMgr -> erase(pmTorIface -> getInfoHash());
		delete this;
	}
	
}

double PeerMonitor::LeechersUpSpeed() 
{
	QMutexLocker lock(&mtx);
	
	double spd = 0.0;
	
	//without it'll segfault/SIGABRT on stop as in meantime the iterator from 
	// mPeers will be invalidated
	
	for( data_t::const_iterator it = mPeers.begin(); it != mPeers.end(); it++)
	{
		if((it != mPeers.end()) && *it && ( (*it) -> getStats().perc_of_file < 100.0) )
		{
			spd += (*it) -> getStats().download_rate;
		}
	}
	
	return spd;
}

double PeerMonitor::LeechersDownSpeed()
{
	QMutexLocker lock(&mtx);
	
	double spd = 0.0;
	
	
	for( data_t::const_iterator it = mPeers.begin(); it != mPeers.end(); it++)
	{
		if((it != mPeers.end()) && *it && ( (*it) -> getStats().perc_of_file < 100.0) )
		{
			spd += (*it) -> getStats().upload_rate;
		}
	}
	
	return spd;

}

double PeerMonitor::SeedersUpSpeed() 
{
	QMutexLocker lock(&mtx);
	
	double spd = 0.0;
	
	
	for( data_t::const_iterator it = mPeers.begin(); it != mPeers.end(); it++)
	{
		if((it != mPeers.end()) && *it && ( (*it) -> getStats().perc_of_file ==  100.0) )
		{
			spd += (*it) -> getStats().download_rate;
		}
		
	}
	
	return spd;

}

uint64_t PeerMonitor::GetLeechers() 
{
	QMutexLocker lock(&mtx);
	
	uint64_t l = 0;
	
	
	for( data_t::const_iterator it = mPeers.begin(); it != mPeers.end(); it++)
	{
		if((it != mPeers.end()) && *it && ( (*it) -> getStats().perc_of_file !=  100.0) )
		{
			l++;
		}
		
	}
	
	return l;
}

uint64_t PeerMonitor::GetSeeders() 
{
	QMutexLocker lock(&mtx);
	
	uint64_t s = 0;
	
	
	for( data_t::const_iterator it = mPeers.begin(); it != mPeers.end(); it++)
	{
		if((it != mPeers.end()) && *it && ( (*it) -> getStats().perc_of_file ==  100) )
		{
			s++;
		}
		
	}
	
	return s;
}

kt::TorrentInterface * PeerMonitor::GetTorIface() const
{
	return pmTorIface;
}

} //NS end
