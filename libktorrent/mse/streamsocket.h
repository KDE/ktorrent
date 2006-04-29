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
#ifndef MSESTREAMSOCKET_H
#define MSESTREAMSOCKET_H


#include <util/constants.h>

class QObject;
class QSocket;
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
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Wrapper around a TCP socket which handles RC4 encryption.
	*/
	class StreamSocket
	{
	public:
		StreamSocket();
		StreamSocket(int fd);
		virtual ~StreamSocket();
		
		/**
		 * Send a chunk of data.
		 * @param data The data
		 * @param len The length
		 */
		void sendData(const Uint8* data,Uint32 len);
		
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
		
		void attachPeer(bt::Peer* peer);
		void detachPeer(bt::Peer* peer);
		void attachAuthenticate(bt::AuthenticateBase* auth);
		void detachAuthenticate(bt::AuthenticateBase* auth);
		void onConnected(QObject* obj,const char* method);
		
		/// Connect the socket to a remote host
		void connectTo(const QString & ip,Uint16 port);
		
		/// Get the IP address of the remote peer
		QString getIPAddress() const;
		
		/**
		 * Reinsert data, this is needed when we read to much during the crypto handshake.
		 * This data will be the first to read out. The data will be copied to a temporary buffer
		 * which will be destroyed when the reinserted data has been read.
		 */
		void reinsert(const Uint8* d,Uint32 size);
		
	private:
		QSocket* sock;
		RC4Encryptor* enc;
		Uint8* reinserted_data;
		Uint32 reinserted_data_size;
		Uint32 reinserted_data_read;
	};

}

#endif
