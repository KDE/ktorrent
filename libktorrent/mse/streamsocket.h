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
#ifndef MSESTREAMSOCKET_H
#define MSESTREAMSOCKET_H

#include <qobject.h>
#include <util/constants.h>
#include <net/bufferedsocket.h>

class QString;

using bt::Uint8;
using bt::Uint16;
using bt::Uint32;

namespace bt
{
	class SHA1Hash;
	class Peer;
	class AuthenticateBase;
}

namespace mse
{
	class RC4Encryptor;
	
	
	

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Wrapper around a TCP socket which handles RC4 encryption.
	 * Once authentication is done, the sendData and readData interfaces should
	 * not be used anymore, a SocketReader and SocketWriter should be provided,
	 * so that reading and writing is controlled from the monitor thread.
	*/
	class StreamSocket : public QObject,public net::SocketReader,public net::SocketWriter
	{
		Q_OBJECT
	public:
		StreamSocket();
		StreamSocket(int fd);
		virtual ~StreamSocket();
		
		/**
		 * Send a chunk of data. (Does not encrypt the data)
		 * @param data The data
		 * @param len The length
		 * @return Number of bytes written
		 */
		Uint32 sendData(const Uint8* data,Uint32 len);
		
		/**
		 * Reads data from the peer.
		 * @param buf The buffer to store the data
		 * @param len The maximum number of bytes to read
		 * @return The number of bytes read
		 */
		Uint32 readData(Uint8* buf,Uint32 len);
		
		/// Get the number of bytes available to read.
		Uint32 bytesAvailable() const;
		
		/// Are we using encryption
		bool encrypted() const {return enc != 0;}
		
		/**
		 * Initialize the RC4 encryption algorithm.
		 * @param dkey 
		 * @param ekey 
		 */
		void initCrypt(const bt::SHA1Hash & dkey,const bt::SHA1Hash & ekey);
		
		/// Set the encryptor
		void setRC4Encryptor(RC4Encryptor* enc);
		
		/// Disables encryption. All data will be sent over as plain text.
		void disableCrypt();
		
		/// Close the socket
		void close();
		
		/// Connect the socket to a remote host
		bool connectTo(const QString & ip,Uint16 port);
		
		/// Get the IP address of the remote peer
		QString getIPAddress() const;
		
		/**
		 * Reinsert data, this is needed when we read to much during the crypto handshake.
		 * This data will be the first to read out. The data will be copied to a temporary buffer
		 * which will be destroyed when the reinserted data has been read.
		 */
		void reinsert(const Uint8* d,Uint32 size);
	
		/// see if the socket is still OK
		bool ok() const;
		
		/// Get the file descriptor
		int fd() const {return sock->fd();}
		
		/// Start monitoring of this socket by the monitor thread
		void startMonitoring(net::SocketReader* rdr,net::SocketWriter* wrt);
		
		/// Is this socket connecting to a remote host
		bool connecting() const;
		
		/// See if a connect was success full
		bool connectSuccesFull() const {return sock->connectSuccesFull();}
		
		/// Get the current download rate
		float getDownloadRate() const;
		
		/// Get the current download rate
		float getUploadRate() const;
	private:
		virtual void onDataReady(Uint8* buf,Uint32 size);
		virtual Uint32 onReadyToWrite(Uint8* data,Uint32 max_to_write);
		virtual bool hasBytesToWrite() const;
		
	private:
		net::BufferedSocket* sock;
		RC4Encryptor* enc;
		Uint8* reinserted_data;
		Uint32 reinserted_data_size;
		Uint32 reinserted_data_read;
		bool monitored;
		net::SocketReader* rdr;
		net::SocketWriter* wrt;
	};

}

#endif
