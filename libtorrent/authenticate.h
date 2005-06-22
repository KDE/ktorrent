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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTAUTHENTICATE_H
#define BTAUTHENTICATE_H

#include <qobject.h>
#include <qsocket.h>
#include <qtimer.h>
#include "globals.h"
#include "sha1hash.h"
#include "peerid.h"


namespace bt
{


	/**
	 * @author Joris Guisson
	 * @brief Authenicate a peer
	 * 
	 * After a peer connects or after we connect to a peer,
	 * we need to authenticate the peer. This class handles this.
	 * It emits the signal finished, when it's done.
	 * This thing times out after 10 secs.
	 */
	class Authenticate : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Directly set the socket. This is used
		 * when somebody connects to us.
		 * @param sock The socket
		 * @param info_hash Info hash
		 * @param peer_id Peer ID
		 */
		Authenticate(QSocket* sock,const SHA1Hash & info_hash,const PeerID & peer_id);

		/**
		 * Connect to a remote host first and authenicate it.
		 * @param ip IP-address of host
		 * @param port Port of host
		 * @param info_hash Info hash
		 * @param peer_id Peer ID
		 */
		Authenticate(const QString & ip,Uint16 port,
					 const SHA1Hash & info_hash,const PeerID & peer_id);
		
		virtual ~Authenticate();

		/**
		 * Get a pointer to the socket, and set it internally
		 * to NULL. After a succesfull authentication, this is used
		 * to transfer ownership to a Peer object.
		 * @return The socket
		 */
		QSocket* takeSocket();
		
		const PeerID & getPeerID() const {return peer_id;}

		/// See if the authentication is finished
		bool isFinished() const {return finished;}

		/// See if the authentication is succesfull
		bool isSuccesfull() const {return succes;}
		
	private slots:
		void connected();
		void error(int err);
		void readyRead();
		void onTimeout();
		
	signals:
		/**
		 * Authenication procedure was finished.
		 * @param obj The Authenicate object 
		 * @param ok Wether or not the authentication procedure was ok
		 */
		//void finished(Authenticate* obj,bool ok);
		
	private:
		void sendHandshake();
		void onFinish(bool succes);
		
		QSocket* sock;
		SHA1Hash info_hash;
		PeerID our_peer_id,peer_id;
		QTimer timer;
		bool done;
		QString host;
		bool finished,succes;
	};

}

#endif
