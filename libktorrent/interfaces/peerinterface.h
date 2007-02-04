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
#ifndef KTPEERINTERFACE_H
#define KTPEERINTERFACE_H

#include <qstring.h>
#include <util/constants.h>
namespace kt
{

	/**
	 * @author Joris Guisson
	 * @brief Interface for a Peer
	 *
	 * This is the interface for a Peer, it allows other classes to
	 * get statistics about a Peer.
	*/
	class PeerInterface
	{
	public:
		PeerInterface();
		virtual ~PeerInterface();

		struct Stats
		{
			/// IP address of peer (dotted notation)
			QString ip_address;
			/// The client (Azureus, BitComet, ...)
			QString client;
			/// Download rate (bytes/s)
			bt::Uint32 download_rate;
			/// Upload rate (bytes/s)
			bt::Uint32 upload_rate;
			/// Choked or not
			bool choked;
			/// Snubbed or not (i.e. we haven't received a piece for a minute)
			bool snubbed;
			/// Percentage of file which the peer has
			float perc_of_file;
			/// Does this peer support DHT
			bool dht_support;
			/// Amount of data uploaded
			bt::Uint64 bytes_uploaded;
			/// Amount of data downloaded
			bt::Uint64 bytes_downloaded;
			/// Advanced choke algorithm score
			double aca_score;
			/// The evil flag is on when the peer has not choked us, 
			/// but has snubbed us and requests have timedout
			bool evil;
			/// Flag to indicate if this peer has an upload slot
			bool has_upload_slot;
			/// Wether or not this connection is encrypted
			bool encrypted;
			/// Number of upload requests queued
			bt::Uint32 num_up_requests;
			/// Number of outstanding download requests queued
			bt::Uint32 num_down_requests;
			/// Supports the fast extensions
			bool fast_extensions;
			/// Is this a peer on the local network
			bool local;
			/// Wether or not the peer supports the extension protocol
			bool extension_protocol;
		};

		virtual const Stats & getStats() const = 0;
		
		virtual void kill() = 0;
	};

}

#endif
