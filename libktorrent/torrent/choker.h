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
#ifndef BTCHOKER_H
#define BTCHOKER_H

#include <qptrlist.h>
#include <util/constants.h>
#include "peer.h"

namespace kt
{
	struct TorrentStats;
}

namespace bt
{
	const Uint32 UNDEFINED_ID = 0xFFFFFFFF;
	
	class PeerManager;
	class ChunkManager;
	
	
	typedef int (*PeerCompareFunc)(Peer* a,Peer* b);
	
	class PeerPtrList : public QPtrList<Peer>
	{
		PeerCompareFunc pcmp;
	public:
		PeerPtrList(PeerCompareFunc pcmp = NULL);
		virtual ~PeerPtrList();
		
		void setCompareFunc(PeerCompareFunc p) {pcmp = p;}
		
		virtual int compareItems(QPtrCollection::Item a, QPtrCollection::Item b);
	};
	
	/**
	 * Base class for all choke algorithms.
	 */
	class ChokeAlgorithm
	{
	protected:
		Uint32 opt_unchoked_peer_id;
	public:
		ChokeAlgorithm();
		virtual ~ChokeAlgorithm();
		
		/**
		 * Do the actual choking when we are still downloading.
		 * @param pman The PeerManager
		 * @param cman The ChunkManager
		 * @param stats The torrent stats
		 */
		virtual void doChokingLeechingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats) = 0;
		
		/**
		 * Do the actual choking when we are seeding
		 * @param pman The PeerManager
		 * @param cman The ChunkManager
		 * @param stats The torrent stats
		 */
		virtual void doChokingSeedingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats) = 0;
		
		/// Get the optimisticly unchoked peer ID
		Uint32 getOptimisticlyUnchokedPeerID() const {return opt_unchoked_peer_id;}
	};

	

	/**
	 * @author Joris Guisson
	 * @brief Handles the choking
	 * 
	 * This class handles the choking and unchoking of Peer's.
	 * This class needs to be updated every 10 seconds.
	*/
	class Choker
	{
		ChokeAlgorithm* choke;
		PeerManager & pman;
		ChunkManager & cman;
		static Uint32 num_upload_slots;
	public:
		Choker(PeerManager & pman,ChunkManager & cman);
		virtual ~Choker();

		/**
		 * Update which peers are choked or not.
		 * @param have_all Indicates wether we have the entire file
		 * @param stats Statistic of the torrent
		 */
		void update(bool have_all,const kt::TorrentStats & stats);

		/// Get the PeerID of the optimisticly unchoked peer.
		Uint32 getOptimisticlyUnchokedPeerID() const {return choke->getOptimisticlyUnchokedPeerID();}
		
		/// Set the number of upload slots
		static void setNumUploadSlots(Uint32 n) {num_upload_slots = n;}
		
		/// Get the number of upload slots
		static Uint32 getNumUploadSlots() {return num_upload_slots;}
	};

}

#endif
