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
#ifndef BTUDPTRACKERSOCKET_H
#define BTUDPTRACKERSOCKET_H

#include <qobject.h>
#include <qmap.h>
#include <qcstring.h>
#include <util/constants.h>


namespace KNetwork
{
	class KDatagramSocket;
	class KSocketAddress;
}

namespace bt
{
	

	enum Action
	{
		CONNECT = 0,
		ANNOUNCE = 1,
		SCRAPE = 2,
		ERROR = 3
	};


	
	/**
	 * @author Joris Guisson
	 *
	 * Class which handles communication with one or more UDP trackers.
	*/
	class UDPTrackerSocket : public QObject
	{
		Q_OBJECT
	public:
		UDPTrackerSocket();
		virtual ~UDPTrackerSocket();

		/**
		 * Send a connect message. As a response to this, the connectRecieved
		 * signal will be emitted, classes recieving this signal should check if
		 * the transaction_id is the same.
		 * @param tid The transaction_id 
		 * @param addr The address to send to
		 */
		void sendConnect(Int32 tid,const KNetwork::KSocketAddress & addr);

		/**
		 * Send an announce message. As a response to this, the announceRecieved
		 * signal will be emitted, classes recieving this signal should check if
		 * the transaction_id is the same.
		 * @param tid The transaction_id
		 * @param data The data to send (connect input structure, in UDP Tracker specifaction)
		 * @param addr The address to send to
		 */
		void sendAnnounce(Int32 tid,const Uint8* data,const KNetwork::KSocketAddress & addr);

		/**
		 * If a transaction times out, this should be used to cancel it.
		 * @param tid 
		 */
		void cancelTransaction(Int32 tid);


		/**
		 * Compute a free transaction_id.
		 * @return A free transaction_id
		 */
		Int32 newTransactionID();
		
		/**
		 * Set the port ot use.
		 * @param p The port
		 */
		static void setPort(Uint16 p);
		
		/// Get the port in use.
		static Uint16 getPort();
	private slots:
		void dataReceived();

	signals:
		/**
		 * Emitted when a connect message is received.
		 * @param tid The transaction_id
		 * @param connection_id The connection_id returned
		 */
		void connectRecieved(Int32 tid,Int64 connection_id);
		
		/**
		 * Emitted when an announce message is received.
		 * @param tid The transaction_id
		 * @param buf The data
		 */
		void announceRecieved(Int32 tid,const QByteArray & buf);

		/**
		 * Signal emitted, when an error occurs during a transaction.
		 * @param tid The transaction_id
		 * @param error_string Potential error string
		 */
		void error(Int32 tid,const QString & error_string);

	private:
		void handleConnect(const QByteArray & buf);
		void handleAnnounce(const QByteArray & buf);
		void handleError(const QByteArray & buf);
		
	private:
		Uint16 udp_port;
		KNetwork::KDatagramSocket* sock;
		QMap<Int32,Action> transactions;
		static Uint16 port;
	};
}

#endif
