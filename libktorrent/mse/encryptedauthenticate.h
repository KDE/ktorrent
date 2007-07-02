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
#ifndef MSEENCRYPTEDAUTHENTICATE_H
#define MSEENCRYPTEDAUTHENTICATE_H

#include <util/sha1hash.h>
#include <torrent/authenticate.h>
#include "bigint.h"


namespace mse
{
	class RC4Encryptor;
	
	const Uint32 MAX_EA_BUF_SIZE = 622 + 512;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Encrypted version of the Authenticate class
	*/
	class EncryptedAuthenticate : public bt::Authenticate
	{
		Q_OBJECT
	public:
		EncryptedAuthenticate(const QString& ip, Uint16 port, const bt::SHA1Hash& info_hash, const bt::PeerID& peer_id, bt::PeerManager* pman);
		virtual ~EncryptedAuthenticate();
		
	private slots:
		virtual void connected();
		virtual void onReadyRead();
		
	private:
		void handleYB();
		void handleCryptoSelect();
		void findVC();
		void handlePadD();
		
	private:
		enum State
		{
			NOT_CONNECTED,
			SENT_YA,
			GOT_YB,
			FOUND_VC,
			WAIT_FOR_PAD_D,
			NORMAL_HANDSHAKE
		};
		
		BigInt xa,ya,s,skey,yb;
		State state; 
		RC4Encryptor* our_rc4;
		Uint8 buf[MAX_EA_BUF_SIZE];
		Uint32 buf_size;
		Uint32 vc_off;
		Uint32 dec_bytes;
		bt::SHA1Hash enc,dec;
		Uint32 crypto_select;
		Uint16 pad_D_len;
		Uint32 end_of_crypto_handshake;
	};

}

#endif
