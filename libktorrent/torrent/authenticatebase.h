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
#ifndef BTAUTHENTICATEBASE_H
#define BTAUTHENTICATEBASE_H

#include <qobject.h>
#include <qsocket.h>
#include <qtimer.h>

#ifdef USE_KNETWORK_SOCKET_CLASSES
namespace KNetwork
{
	class KBufferedSocket;
}
#endif

namespace bt
{
	class SHA1Hash;
	class PeerID;

	/**
	 * @author Joris Guisson
	 *
	 * Base class for authentication classes. This class just groups
	 * some common stuff between Authenticate and ServerAuthentciate.
	 * It has a socket, handles the timing out, provides a function to send
	 * the handshake.
	 */
	class AuthenticateBase : public QObject
	{
		Q_OBJECT
	public:
#ifdef USE_KNETWORK_SOCKET_CLASSES
		AuthenticateBase(KNetwork::KBufferedSocket* s = 0);
#else
		AuthenticateBase(QSocket* s = 0);
#endif
		virtual ~AuthenticateBase();

		/// See if the authentication is finished
		bool isFinished() const {return finished;}
		
	protected:
		/**
		 * Send a handshake
		 * @param info_hash The info_hash to include 
		 * @param our_peer_id Our PeerID
		 */
		void sendHandshake(const SHA1Hash & info_hash,const PeerID & our_peer_id);
		
		/**
		 * Authentication finished.
		 * @param succes Succes or not
		 */
		virtual void onFinish(bool succes) = 0;
		
		/**
		 * The other side send a handshake. The first 20 bytes
		 * of the handshake will allready have been checked.
		 * @param full Indicates wether we have a full handshake
		 *  if this is not full, we should just send our own
		 */
		virtual void handshakeRecieved(bool full) = 0;
		
	private slots:
		void onTimeout();
		void onError(int err);
		void onReadyRead();

	protected:
#ifdef USE_KNETWORK_SOCKET_CLASSES
		KNetwork::KBufferedSocket* sock;
#else
		QSocket* sock;
#endif
		QTimer timer;
		bool finished;
		Uint8 handshake[68];
		Uint32 bytes_of_handshake_recieved;
	};

}

#endif
