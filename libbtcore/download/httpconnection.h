/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef BTHTTPCONNECTION_H
#define BTHTTPCONNECTION_H

#include <QMutex>
#include <QHttpRequestHeader>
#include <k3resolver.h>
#include <net/bufferedsocket.h>

class KUrl;

namespace bt
{

	/**
		@author Joris Guisson
		
		HTTP connection for webseeding. We do not use KIO here, because we want to be able to apply 
		the maximum upload and download rate to webseeds;
	*/
	class HttpConnection : public QObject,public net::SocketReader,public net::SocketWriter
	{
		Q_OBJECT
	private:
		enum State
		{
			IDLE,RESOLVING,CONNECTING,ACTIVE,ERROR
		};
		
		struct HttpGet
		{
			QString path;
			bt::Uint64 start;
			bt::Uint64 len;
			bt::Uint64 data_received;
			QByteArray buffer;
			bt::Uint32 bytes_sent;
			QList<QByteArray> piece_data;
			bool response_header_received;
			bool request_sent;
			
			HttpGet(const QString & path,bt::Uint64 start,bt::Uint64 len);
			virtual ~HttpGet();
			
			bool onDataReady(Uint8* buf,Uint32 size);
		};
		
		net::BufferedSocket* sock;
		State state;
		mutable QMutex mutex;
		QList<HttpGet*> requests;
	public:
		HttpConnection();
		virtual ~HttpConnection();
		
		/**
		 * Connect to a webseed
		 * @param url Url of the webseeder
		 */
		void connectTo(const KUrl & url);
		
		/// Check if the connection is OK
		bool ok() const;
		
		/// See if we are connected
		bool connected() const;
		
		/**
		 * Do a HTTP GET request
		 * @param path The path of the file
		 * @param start Offset into file
		 * @param len Length of data to download
		 */
		bool get(const QString & path,bt::Uint64 start,bt::Uint64 len);

		virtual void onDataReady(Uint8* buf,Uint32 size);
		virtual Uint32 onReadyToWrite(Uint8* data,Uint32 max_to_write);	
		virtual bool hasBytesToWrite() const;
		
		/**
		 * Get some part of the 
		 * @param data Bytearray to copy the data into
		 * @return true if data was filled in
		 */
		bool getData(QByteArray & data);
		
		/// Get the current download rate
		float getDownloadRate() const;
		
	private slots:
		void hostResolved(KNetwork::KResolverResults res);

	};
}

#endif
