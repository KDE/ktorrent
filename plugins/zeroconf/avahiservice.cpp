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
#include <util/log.h>
#include <torrent/peerid.h>
#include "localbrowser.h"
#include "avahiservice.h"
		
using namespace bt;
		
namespace kt
{
	
	void group_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, void* userdata)
	{
		AvahiService* service = reinterpret_cast<AvahiService*>(userdata);
	
		if (g == service->group) {
			switch (state) {
			case AVAHI_ENTRY_GROUP_ESTABLISHED:
				break;
			case AVAHI_ENTRY_GROUP_COLLISION:
				{
					Out(SYS_ZCO|LOG_DEBUG) << "ZC: Entry group collision." << endl;
					avahi_threaded_poll_stop(service->publisher_poll);
					break;
				}
			case AVAHI_ENTRY_GROUP_FAILURE:
				{
					Out(SYS_ZCO|LOG_DEBUG) <<  "ZC: Entry group failure." << endl;
					avahi_threaded_poll_stop(service->publisher_poll);
					break;
				}
			case AVAHI_ENTRY_GROUP_UNCOMMITED:
				{
					Out(SYS_ZCO|LOG_DEBUG) <<  "ZC: Entry group uncommited." << endl;
					break;
				}
			case AVAHI_ENTRY_GROUP_REGISTERING:
				;
			}
		}
	}
	
	void publish_service(AvahiService* service, AvahiClient *c)
	{
		assert(c);
	
		if (!(service->group)) {
			if (!(service->group = avahi_entry_group_new(c, group_callback, service))) {
				Out(SYS_ZCO|LOG_DEBUG) << "ZC: avahi_entry_group_new failed." << endl;
				avahi_threaded_poll_stop(service->publisher_poll);
				return;
			}
		}
	
		const char* name    = avahi_strdup((QString("%1__%2%3").arg(service->id).arg((rand() % 26) + 65).arg((rand() % 26) + 65).arg((rand() % 26) + 65)).ascii());
		const char* type    = avahi_strdup("_bittorrent._tcp");
		const char* subtype = avahi_strdup(QString("_" + service->infoHash + "._sub._bittorrent._tcp").ascii());
	
		if (avahi_entry_group_add_service(
				service->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
				(AvahiPublishFlags)0, name, type, NULL, NULL, service->port, NULL)) {
			if (avahi_client_errno(c) != -8) {
				Out(SYS_ZCO|LOG_DEBUG) << QString("ZC: Failed to add the service (%i).").arg(avahi_client_errno(c)) << endl;
				avahi_threaded_poll_stop(service->publisher_poll);
			} else
				publish_service(service, c);
			return;
		}
	
		if (avahi_entry_group_add_service_subtype(
				service->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
				(AvahiPublishFlags)0, name, type, NULL, subtype)) {
			Out(SYS_ZCO|LOG_DEBUG) << QString("ZC: Failed to add the service subtype (%i).").arg( avahi_client_errno(c)) << endl;
			avahi_threaded_poll_stop(service->publisher_poll);
			return;
		}
	
		if (avahi_entry_group_commit(service->group)) {
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to commit the entry group." << endl;
			avahi_threaded_poll_stop(service->publisher_poll);
			return;
		}
	}
	
	void publisher_callback(AvahiClient* c, AvahiClientState state, void* userdata)
	{
		if (!c)
			return;
	
		AvahiService* service = reinterpret_cast<AvahiService*>(userdata);
	
		switch (state) {
		case AVAHI_CLIENT_S_RUNNING:
			{
				if (!service->group)
					publish_service(service, c);
				break;
			}
		case AVAHI_CLIENT_FAILURE:
			{
				Out(SYS_ZCO|LOG_DEBUG) << "Failure when publishing." << endl;
				avahi_threaded_poll_stop(service->publisher_poll);
				break;
			}
		case AVAHI_CLIENT_S_COLLISION:
		case AVAHI_CLIENT_S_REGISTERING:
			{
				if (service->group)
					avahi_entry_group_reset(service->group);
				break;
			}
		case AVAHI_CLIENT_CONNECTING:
			;
		}
	}
	
	void listener_callback(AvahiClient* c, AvahiClientState state, void* userdata)
	{
		assert(c);
	
		AvahiService* service = reinterpret_cast<AvahiService*>(userdata);
		
		if (state == AVAHI_CLIENT_FAILURE) {
			Out(SYS_ZCO|LOG_DEBUG) <<  "ZC: Server connection failure." << endl;
			avahi_threaded_poll_stop(service->listener_poll);
		}
	}
	
	void resolve_callback(
		AvahiServiceResolver* r,
		AVAHI_GCC_UNUSED AvahiIfIndex interface,
		AVAHI_GCC_UNUSED AvahiProtocol protocol,
		AvahiResolverEvent event,
		const char* name,
		AVAHI_GCC_UNUSED const char* type,
		AVAHI_GCC_UNUSED const char* domain,
		AVAHI_GCC_UNUSED const char* host_name,
		const AvahiAddress* address,
		uint16_t port,
		AVAHI_GCC_UNUSED AvahiStringList* txt,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		void* userdata)
	{
		assert(r);
	
		switch (event) {
		case AVAHI_RESOLVER_FAILURE:
			{
				Out(SYS_ZCO|LOG_DEBUG) <<  "ZC: Resolver failed." << endl;
				break;
			}
		case AVAHI_RESOLVER_FOUND:
			{
				AvahiService* service = reinterpret_cast<AvahiService*>(userdata);
				
				QString realname = QString(name);
				realname.truncate(realname.length() - 5);
	
				if (service->id != QString(realname)) 
				{
					char a[AVAHI_ADDRESS_STR_MAX];
					avahi_address_snprint(a, sizeof(a), address);
					const char* ip = a;
					LocalBrowser::insert(bt::PeerID(realname.ascii()));
	
					Out(SYS_ZCO|LOG_NOTICE) << "ZC: found local peer " << ip << ":" << port << endl;
					service->addPeer(ip,port,true);
					service->emitPeersReady(); 
				}
			}
		}
		avahi_service_resolver_free(r);
	}
	
	void browser_callback(
		AvahiServiceBrowser* b,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiBrowserEvent event,
		const char* name,
		const char* type,
		const char* domain,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		void* userdata)
	{
		assert(b);
	
		AvahiService* service = reinterpret_cast<AvahiService*>(userdata);
	
		switch (event) {
		case AVAHI_BROWSER_FAILURE:
			{
				Out(SYS_ZCO|LOG_DEBUG) << "ZC: Browser failure." << endl;
				avahi_threaded_poll_stop(service->listener_poll);
				break;
			}
		case AVAHI_BROWSER_NEW:
			{
				if (!(avahi_service_resolver_new(service->listener, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback, userdata)))
					Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to resolve the service." << endl;
				break;
			}
		case AVAHI_BROWSER_REMOVE:
			{
				QString realname = QString(name);
				realname.truncate(realname.length() - 5);
	
				LocalBrowser::remove(bt::PeerID(realname.ascii()));
	
				Out(SYS_ZCO|LOG_DEBUG) << "ZC: Browser removed." << endl;
				break;
			}
		case AVAHI_BROWSER_ALL_FOR_NOW:
		case AVAHI_BROWSER_CACHE_EXHAUSTED:
			;
		}
	}
	
	AvahiService::AvahiService(const bt::PeerID& id,bt::Uint16 port, const bt::SHA1Hash & infoHash)
		: group(0), publisher_poll(0), listener_poll(0), 
		publisher(0), listener(0), browser(0)
	{ 
		started = false; 
	
		this->id = QString(id.toString());
		this->port = port;
		this->infoHash = QString(infoHash.toString());
	}
	
	AvahiService::~AvahiService() 
	{
	}
	
	void AvahiService::stop(bt::WaitJob*)
	{
		if (started) 
		{
			started = false;
	
			if (this->publisher_poll)
				avahi_threaded_poll_stop(this->publisher_poll);
	
			if (this->publisher)
				avahi_client_free(this->publisher);
	
			if (this->listener_poll)
				avahi_threaded_poll_stop(this->listener_poll);
	
			if (this->listener)
				avahi_client_free(this->listener);
		}
	}
	
	void AvahiService::start()
	{
		startPublishing();
		startBrowsing();
	
		started = true;
	}
	
	void AvahiService::startPublishing()
	{
		this->group = NULL;
		this->publisher_poll = NULL;
		this->publisher = NULL;
	
		if (!(this->publisher_poll = avahi_threaded_poll_new())) 
		{
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to create a poll for publishing." << endl;
			stop(); 
			return;
		}
	
		this->publisher = avahi_client_new(avahi_threaded_poll_get(publisher_poll), AVAHI_CLIENT_NO_FAIL, publisher_callback, this, NULL);
	
		if (!(this->publisher)) 
		{
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to create a client for publishing." << endl;
			stop(); 
			return;
		}
	
		avahi_threaded_poll_start(publisher_poll);
	}
	
	void AvahiService::startBrowsing()
	{
		this->listener_poll = NULL;
		this->listener = NULL;
		this->browser = NULL;
		
		if (!(this->listener_poll = avahi_threaded_poll_new())) 
		{
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to create a poll for browsing." << endl;
			stop(); 
			return;
		}
		
		this->listener = avahi_client_new(avahi_threaded_poll_get(this->listener_poll), AVAHI_CLIENT_NO_FAIL, listener_callback, this, NULL);
		
		if (!(this->listener)) 
		{
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to create a client for browsing." << endl;
			stop(); 
			return;
		}
		
		if (!(this->browser = avahi_service_browser_new(this->listener, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, avahi_strdup(QString("_" + this->infoHash + "._sub._bittorrent._tcp").ascii()), NULL, (AvahiLookupFlags)0, browser_callback, this))) 
		{
			Out(SYS_ZCO|LOG_DEBUG) << "ZC: Failed to create a service browser." << endl;
			stop(); 
			return;
		}
	
		avahi_threaded_poll_start(listener_poll);
	}
	
	void AvahiService::emitPeersReady()
	{
		peersReady(this);
	}
	
	void AvahiService::aboutToBeDestroyed()
	{
		serviceDestroyed(this);
	}
}

#include "avahiservice.moc"
