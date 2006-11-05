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
#ifndef MSEENCRYPTEDSERVERAUTHENTICATE_H
#define MSEENCRYPTEDSERVERAUTHENTICATE_H

#include <util/sha1hash.h>
#include <torrent/serverauthenticate.h>
#include "bigint.h"

namespace mse
{
	class RC4Encryptor;
	

	const Uint32 MAX_SEA_BUF_SIZE = 608 + 20 + 20 + 8 + 4 + 2 + 512 + 2 + 68;
	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class EncryptedServerAuthenticate : public bt::ServerAuthenticate
	{
		Q_OBJECT
	public:
		EncryptedServerAuthenticate(mse::StreamSocket* sock, bt::Server* server);
		virtual ~EncryptedServerAuthenticate();

	private slots:
		virtual void onReadyRead();
		
	private:
		void handleYA();
		void sendYB();
		void findReq1();
		void calculateSKey();
		void processVC();
		void handlePadC();
		void handleIA();
		
	private:
		enum State
		{
			WAITING_FOR_YA,
			WAITING_FOR_REQ1,
			FOUND_REQ1,
			FOUND_INFO_HASH,
			WAIT_FOR_PAD_C,
			WAIT_FOR_IA,
			NON_ENCRYPTED_HANDSHAKE
		};
		BigInt xb,yb,s,ya;
		bt::SHA1Hash skey,info_hash;
		State state;
		Uint8 buf[MAX_SEA_BUF_SIZE];
		Uint32 buf_size;
		Uint32 req1_off;
		Uint32 crypto_provide,crypto_select;
		Uint16 pad_C_len;
		Uint16 ia_len;
		RC4Encryptor* our_rc4;
	};

}

#endif
