/***************************************************************************
 *   Copyright (C) 2006 by Lesly Weyts and Kevin Andre                     *
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
#ifndef AVAHI_SERVICE_HH
#define AVAHI_SERVICE_HH

/**
 * @author Lesly Weyts and Kevin Andre
 * @brief Handles everything directly related to Avahi
 *
 * This set of functions provide a simple way to use Avahi inside the KTorrent source code.
 */

#include <cstdlib>
#include <qstring.h>


#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>

#include <util/sha1hash.h>
#include <interfaces/peersource.h>

namespace kt
{

	class AvahiService : public kt::PeerSource
	{
		Q_OBJECT
	public:
		AvahiService(const bt::PeerID&, bt::Uint16, const bt::SHA1Hash&);
		virtual ~AvahiService();
		
		virtual void stop(bt::WaitJob* wjob = 0);
		virtual void start();
		virtual void aboutToBeDestroyed();
		
		void emitPeersReady();
		
	signals:
		void serviceDestroyed(AvahiService* av);
	
	private:
		bool startPublishing();
		bool startBrowsing();
		
		friend void group_callback(AvahiEntryGroup*, AvahiEntryGroupState, void*);
		friend void publish_service(AvahiService*, AvahiClient*);
		friend void publisher_callback(AvahiClient*, AvahiClientState, void*);
		friend void listener_callback(AvahiClient*, AvahiClientState, void*);
		
		friend void resolve_callback(
			AvahiServiceResolver*,
			AvahiIfIndex,
			AvahiProtocol,
			AvahiResolverEvent,
			const char*,
			const char*,
			const char*,
			const char*,
			const AvahiAddress*,
			uint16_t,
			AvahiStringList*,
			AvahiLookupResultFlags,
			void*
		);
	
		friend void browser_callback(
			AvahiServiceBrowser*,
			AvahiIfIndex,
			AvahiProtocol,
			AvahiBrowserEvent,
			const char*,
			const char*,
			const char*,
			AvahiLookupResultFlags,
			void*
		);
	
		QString id;
		int port;
		QString infoHash;
	
		bool started;
	
		AvahiEntryGroup *group;
		const AvahiPoll* publisher_poll;
		const AvahiPoll* listener_poll;
		AvahiClient* publisher;
		AvahiClient* listener;
		AvahiServiceBrowser *browser;
	};
}

#endif
