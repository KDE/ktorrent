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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTCHOKER_H
#define BTCHOKER_H

#include <qptrlist.h>
#include <util/constants.h>
#include "peer.h"

namespace bt
{
	class PeerManager;
	
	class PeerPtrList : public QPtrList<Peer>
	{
		bool dl_cmp;
	public:
		PeerPtrList(bool dl_cmp = true);
		virtual ~PeerPtrList();
		
		void setDLCmp(bool dl)
		{
			dl_cmp = dl;
		}
	
		virtual int compareItems(QPtrCollection::Item a, QPtrCollection::Item b);
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
		PeerManager & pman;
		int opt_unchoke_index;
		int opt_unchoke;
		Uint32 opt_unchoked_peer_id;
		PeerPtrList downloaders,interested,not_interested;
	public:
		Choker(PeerManager & pman);
		virtual ~Choker();

		/**
		 * Update which peers are choked or not.
		 * @param have_all Indicates wether we have the entire file
		 */
		void update(bool have_all);

		/// Get the PeerID of the optimisticly unchoked peer.
		Uint32 getOptimisticlyUnchokedPeerID() const {return opt_unchoked_peer_id;}

	private:
		void updateInterested();
		void updateDownloaders();
		void sendInterested(bool have_all);
		void sendUnchokes(bool have_all);
		void optimisticUnchoke();
	};

}

#endif
