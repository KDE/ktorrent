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
#ifndef BTAUTHENTICATE_H
#define BTAUTHENTICATE_H


#include <util/sha1hash.h>
#include <peer/peerid.h>
#include "authenticatebase.h"

namespace net
{
	class Socks;
}

namespace bt
{

	
	class PeerConnector;


	/**
	 * @author Joris Guisson
	 * @brief Authenicate a peer
	 * 
	 * After we connect to a peer,
	 * we need to authenticate the peer. This class handles this.
	 */
	class Authenticate : public AuthenticateBase
	{
		Q_OBJECT
	public:
		/**
		 * Connect to a remote host first and authenicate it.
		 * @param ip IP-address of host
		 * @param port Port of host
		 * @param proto Transport protocol to use
		 * @param info_hash Info hash
		 * @param peer_id Peer ID
		 * @param pman PeerManager
		 */
		Authenticate(const QString & ip,Uint16 port,TransportProtocol proto,
					 const SHA1Hash & info_hash,const PeerID & peer_id,
					 PeerConnector* pcon);
		
		virtual ~Authenticate();

		/**
		 * Get a pointer to the socket, and set it internally
		 * to NULL. After a succesfull authentication, this is used
		 * to transfer ownership to a Peer object.
		 * @return The socket
		 */
		mse::StreamSocket* takeSocket();
		
		const PeerID & getPeerID() const {return peer_id;}

		/// See if the authentication is succesfull
		bool isSuccesfull() const {return succes;}
		
		const QString & getIP() const {return host;}
		Uint16 getPort() const {return port;}
		
	public slots:
		/// Stop the authentication
		void stop();
		
	protected slots:
		virtual void onReadyWrite();
		virtual void onReadyRead();
		
	protected:
		void onFinish(bool succes);
		void handshakeReceived(bool full);
		virtual void connected();
		
	protected:
		SHA1Hash info_hash;
		PeerID our_peer_id,peer_id;
		QString host;
		Uint16 port;
		bool succes;
		PeerConnector* pcon;
		net::Socks* socks;
	};
}

#endif
