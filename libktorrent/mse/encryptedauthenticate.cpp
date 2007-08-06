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
#include <stdlib.h>
#include <algorithm>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include "encryptedauthenticate.h"
#include "rc4encryptor.h"
#include "streamsocket.h"
#include "functions.h"

using namespace bt;

namespace mse
{
	
	

	EncryptedAuthenticate::EncryptedAuthenticate(
			const QString& ip, 
			Uint16 port, 
			const SHA1Hash& info_hash, 
			const PeerID& peer_id, 
			PeerManager* pman)
	: Authenticate(ip, port, info_hash, peer_id, pman)
	{
		mse::GeneratePublicPrivateKey(xa,ya);
		state = NOT_CONNECTED;
		buf_size = 0;
		our_rc4 = 0;
		vc_off = 0;
		dec_bytes = 0;
		crypto_select = 0;
		pad_D_len = 0;
		end_of_crypto_handshake = 0;
		//Out(SYS_CON|LOG_DEBUG) << "EncryptedAuthenticate : " << ip << ":" << port << endl;
	}


	EncryptedAuthenticate::~EncryptedAuthenticate()
	{
		delete our_rc4;
	}
	
	
	
	void EncryptedAuthenticate::connected()
	{
		// we are connected so send ya and some padding
		Uint8 tmp[608];
		ya.toBuffer(tmp,96);
		sock->sendData(tmp,96 + rand() % 512);
		state = SENT_YA;
	}
	
	/*
	1 A->B: Diffie Hellman Ya, PadA
	2 B->A: Diffie Hellman Yb, PadB
	3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
	4 B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
	5 A->B: ENCRYPT2(Payload Stream)
	*/
	
	
	
	void EncryptedAuthenticate::handleYB()
	{
		// if you can't sent 96 bytes you are not worth the effort
		if (buf_size < 96)
		{
			Out(SYS_CON|LOG_DEBUG) << "Not enough data received, encrypted authentication failed" << endl;
			onFinish(false);
			return;
		}
		
		// read Yb
		yb = BigInt::fromBuffer(buf,96);
		
		// calculate s
		s = mse::DHSecret(xa,yb);
		
		state = GOT_YB;
		// now we must send line 3 
		Uint8 tmp_buf[120]; // temporary buffer
		bt::SHA1Hash h1,h2; // temporary hash
		
		// generate and send the first hash
		memcpy(tmp_buf,"req1",4);
		s.toBuffer(tmp_buf + 4,96);
		h1 = SHA1Hash::generate(tmp_buf,100);
		sock->sendData(h1.getData(),20);
		
		// generate second and third hash and xor them
		memcpy(tmp_buf,"req2",4);
		memcpy(tmp_buf+4,info_hash.getData(),20);
		h1 = SHA1Hash::generate(tmp_buf,24);
		
		memcpy(tmp_buf,"req3",4);
		s.toBuffer(tmp_buf + 4,96);
		h2 = SHA1Hash::generate(tmp_buf,100);
		sock->sendData((h1 ^ h2).getData(),20);
		
		// now we enter encrypted mode the keys are :
		// HASH('keyA', S, SKEY) for the encryption key
		// HASH('keyB', S, SKEY) for the decryption key
		enc = mse::EncryptionKey(true,s,info_hash);
		dec = mse::EncryptionKey(false,s,info_hash);
		
		our_rc4 = new RC4Encryptor(dec,enc);
		
		// now we must send ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA))
		memset(tmp_buf,0,16); // VC are 8 0x00's
		if (Globals::instance().getServer().unencryptedConnectionsAllowed())
			tmp_buf[11] = 0x03; // we support both plain text and rc4
		else
			tmp_buf[11] = 0x02;
		WriteUint16(tmp_buf,12,0x0000); // no padC
		WriteUint16(tmp_buf,14,68); // length of IA, which will be the bittorrent handshake
		// send IA which is the handshake
		makeHandshake(tmp_buf+16,info_hash,our_peer_id);
		sock->sendData(our_rc4->encrypt(tmp_buf,84),84); 
		
		// search for the encrypted VC in the data
		findVC();
	}
	
	void EncryptedAuthenticate::findVC()
	{
		Uint8 vc[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		
		RC4Encryptor rc4(enc,dec);
		memcpy(vc,rc4.encrypt(vc,8),8);
		
		Uint32 max_i = buf_size - 8;		
		for (Uint32 i = 96;i < max_i;i++)
		{	
			if (vc[0] == buf[i] && memcmp(buf+i,vc,8) == 0)
			{
				state = FOUND_VC;
				vc_off = i;
				handleCryptoSelect();
				return;
			}
		}
	
		// we haven't found it in the first 616 bytes (96 + max 512 padding + 8 bytes VC)
		if (buf_size >= 616)
		{
			onFinish(false);
		}
	}
	
	void EncryptedAuthenticate::handleCryptoSelect()
	{
		// not enough data available so lets come back later
		if (vc_off + 14 >= buf_size)
			return;
		
		// now decrypt the first 14 bytes
		our_rc4->decrypt(buf + vc_off,14);
		// check the VC
		for (Uint32 i = vc_off;i < vc_off + 8;i++)
		{
			if (buf[i])
			{
				Out(SYS_CON|LOG_DEBUG) << "Invalid VC " << endl;
				onFinish(false);
				return;
			}
		}
		
		crypto_select = ReadUint32(buf,vc_off + 8);
		pad_D_len = ReadUint16(buf,vc_off + 12);
		if (pad_D_len > 512)
		{
			Out(SYS_CON|LOG_DEBUG) << "Invalid pad D length" << endl;
			onFinish(false);
			return;
		}
		
		end_of_crypto_handshake = vc_off + 14 + pad_D_len;
		if (!(vc_off + 14 + pad_D_len < buf_size))
		{
			// padD is not complete, wait for that
			state = WAIT_FOR_PAD_D;
			return;
		}
		
		handlePadD();
	}
	
	void EncryptedAuthenticate::handlePadD()
	{
		// decrypt the padding
		our_rc4->decrypt(buf + (vc_off + 14),pad_D_len);
		
		bool rc4 = false;
		if (crypto_select & 0x00000001) // plain_text selected
		{
			delete our_rc4;
			our_rc4 = 0;
		}
		else if (crypto_select & 0x00000002) // now it must be rc4 if not exit
		{
			sock->setRC4Encryptor(our_rc4);
			our_rc4 = 0;
			rc4 = true;
		}
		else // we don't support anything else so error out
		{
			onFinish(false);
			return;
		}
		
		// noz we wait for the normal handshake
		state = NORMAL_HANDSHAKE;
		// if we have read more then the crypto handshake, reinsert it
		if (buf_size > vc_off + 14 + pad_D_len)
		{
			Uint32 off = vc_off + 14 + pad_D_len;
			sock->reinsert(buf + off,buf_size - off);
			Authenticate::onReadyRead();
		}
	}
	
	void EncryptedAuthenticate::onReadyRead()
	{
		if (finished)
			return;
		
	
		Uint32 ba = sock->bytesAvailable();
		if (ba == 0)
		{
			onFinish(false);
			return;
		}
		
		if (state != NORMAL_HANDSHAKE)
		{
			if (buf_size + ba > MAX_EA_BUF_SIZE)
				ba = MAX_EA_BUF_SIZE - buf_size;
			
			// do not read past the end of padD
			if (pad_D_len > 0 && buf_size + ba > vc_off + 14 + pad_D_len)
				ba = (vc_off + 14 + pad_D_len) - buf_size;
			// read data
			buf_size += sock->readData(buf + buf_size,ba);
			
		}
		
		switch (state)
		{
		case SENT_YA:
			if (ba > 608)
			{
				onFinish(false);
			}
			else
			{
				handleYB();
			}
			break;
		case GOT_YB:
			findVC();
			break;
		case FOUND_VC:
			handleCryptoSelect();
			break;
		case WAIT_FOR_PAD_D:
			handlePadD();
			break;
		case NORMAL_HANDSHAKE:
			// let AuthenticateBase deal with the data
			AuthenticateBase::onReadyRead();
			break;
		};
	}


}

#include "encryptedauthenticate.moc"
