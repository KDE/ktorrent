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

#ifndef PEERMONITOR_H_
#define PEERMONITOR_H_

#include <qmutex.h>

#include <interfaces/monitorinterface.h>
#include <interfaces/peerinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/sha1hash.h>

#include <map>
#include <list>
#include <algorithm>

namespace kt {

/**
\brief Monitors peers
\author Krzysztof Kundzicz <athantor@gmail.com>

Used for peers statistics

\warning Don't use it, as There Can Be Only One™ and the infowidgetplugin relays on it
*/

class PeerMonitor : public MonitorInterface, public QObject
{
	public:
		///Type of conteiner of pointers to peers
		typedef std::list<PeerInterface *> data_t;
	
	private:
		/**
		\brief Mutex
		
		Used for locking conteiner with pointers to peers
		*/
		QMutex mtx;
		///Container with pointers to peers
		data_t mPeers;
		///Monitored torrent
		TorrentInterface * pmTorIface;
		/**
		\brief Pointer to PeerMonitor „manager”
		
		\li \c Key: Monitored torrent hash
		\li \c Value \c type: Pointer to peer monitor
		*/
		std::map<bt::SHA1Hash, PeerMonitor *> *pmPeerMMgr;
	
	public:
		/**
		\brief Constructor
		\param pTi Pointer to monitored torrent
		\param pM Pointer to PeerMonitor „manager”
		*/
		PeerMonitor(TorrentInterface * pTi, std::map<bt::SHA1Hash, PeerMonitor *> * pM);
		virtual ~PeerMonitor();
		
		virtual void peerAdded (PeerInterface *peer);
		virtual void peerRemoved (PeerInterface *peer);
		virtual void downloadStarted (ChunkDownloadInterface *cd);
		virtual void downloadRemoved (ChunkDownloadInterface *cd);
		virtual void stopped();
		virtual void destroyed () ;
		
		/**
		\brief Gets speed @ which leechers are uploading to us
		\return Speed
		*/
		double LeechersUpSpeed() ;
		/**
		\brief Gets speed @ which leechers are downloading from us
		\return Speed
		*/
		double LeechersDownSpeed();
		/**
		\brief Gets speed @ which seeders are uploading to us
		\return Speed
		*/
		double SeedersUpSpeed() ;
		/**
		\brief  Gets leechers count to which we are connected
		\return Count
		*/
		uint64_t GetLeechers() ;
		/**
		\brief Gets seeders count to which we are connected
		\return Count
		*/
		uint64_t GetSeeders() ;
		
		/**
		\brief Gets pointer to monitored torrent
		\return Pointer to monitored torrent
		*/
		TorrentInterface * GetTorIface() const;
	
};

}

#endif
