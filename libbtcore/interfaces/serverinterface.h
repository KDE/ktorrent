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

#ifndef BT_SERVERINTERFACE_H
#define BT_SERVERINTERFACE_H

#include <QObject>
#include <QStringList>
#include <btcore_export.h>
#include <util/constants.h>

namespace mse
{
	class StreamSocket;
}

namespace bt
{
	class SHA1Hash;
	class PeerManager;
	
	/**
		Base class for all servers which accept connections.
	*/
	class BTCORE_EXPORT ServerInterface : public QObject
	{
		Q_OBJECT
	public:
		ServerInterface(QObject* parent = 0);
		virtual ~ServerInterface();
		
		
		/**
		* Change the port.
		* @param port The new port
		*/
		virtual bool changePort(Uint16 port) = 0;
		
		/// Set the port to use
		static void setPort(Uint16 p) {port = p;}
		
		/// Get the port in use
		static Uint16 getPort() {return port;}
		
		/**
		 * Add a PeerManager.
		 * @param pman The PeerManager
		 */
		static void addPeerManager(PeerManager* pman);
		
		/**
		* Remove a PeerManager.
		* @param pman The PeerManager
		*/
		static void removePeerManager(PeerManager* pman);
		
		/**
		* Find the PeerManager given the info_hash of it's torrent.
		* @param hash The info_hash
		* @return The PeerManager or 0 if one can't be found
		*/
		static PeerManager* findPeerManager(const SHA1Hash & hash);
		
		/**
		* Find the info_hash based on the skey hash. The skey hash is a hash
		* of 'req2' followed by the info_hash. This function finds the info_hash
		* which matches the skey hash.
		* @param skey HASH('req2',info_hash)
		* @param info_hash which matches
		* @return true If one was found
		*/
		static bool findInfoHash(const SHA1Hash & skey,SHA1Hash & info_hash);
		
		/**
		* Enable encryption. 
		* @param allow_unencrypted Allow unencrypted connections (if encryption fails)
		*/
		static void enableEncryption(bool allow_unencrypted);
		
		/**
		* Disable encrypted authentication.
		*/
		static void disableEncryption();
		
		static bool isEncryptionEnabled() {return encryption;}
		static bool unencryptedConnectionsAllowed() {return allow_unencrypted;}
		
		/**
			Get a list of potential IP addresses to bind to
		*/
		static QStringList bindAddresses();
		
		static void setUtpEnabled(bool on,bool only_use_utp);
		static bool isUtpEnabled() {return utp_enabled;}
		static bool onlyUseUtp() {return only_use_utp;}
		static void setPrimaryTransportProtocol(TransportProtocol proto);
		static TransportProtocol primaryTransportProtocol() {return primary_transport_protocol;}
		
	protected:
		void newConnection(mse::StreamSocket* sock);
		
	protected:
		static Uint16 port;
		static QList<PeerManager*> peer_managers;
		static bool encryption;
		static bool allow_unencrypted;
		static bool utp_enabled;
		static bool only_use_utp;
		static TransportProtocol primary_transport_protocol;
	};

}

#endif // BT_SERVERINTERFACE_H
