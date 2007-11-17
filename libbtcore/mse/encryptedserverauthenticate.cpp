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
#include <util/functions.h>
#include <util/log.h>
#include <torrent/server.h>
#include <torrent/globals.h>
#include "encryptedserverauthenticate.h"
#include "functions.h"
#include "streamsocket.h"
#include "rc4encryptor.h"

using namespace bt;

namespace mse
{
	EncryptedServerAuthenticate::EncryptedServerAuthenticate(mse::StreamSocket* sock, bt::Server* server): bt::ServerAuthenticate(sock, server)
	{
		mse::GeneratePublicPrivateKey(xb,yb);
		state = WAITING_FOR_YA;
		buf_size = 0;
		req1_off = 0;
		our_rc4 = 0;
		pad_C_len = 0;
		crypto_provide = crypto_select = 0;
	}


	EncryptedServerAuthenticate::~EncryptedServerAuthenticate()
	{
		delete our_rc4;
	}
	
	void EncryptedServerAuthenticate::sendYB()
	{
		Uint8 tmp[608];
		yb.toBuffer(tmp,96);
	//	DumpBigInt("Xb",xb);
	//	DumpBigInt("Yb",yb);
		sock->sendData(tmp,96 + rand() % 512);
		//Out() << "Sent YB" << endl;
	}
	
	
	void EncryptedServerAuthenticate::handleYA()
	{
		sendYB();
		
		ya = BigInt::fromBuffer(buf,96);
	//	DumpBigInt("Ya",ya);
		// now calculate secret 
		s = mse::DHSecret(xb,ya);
	//	DumpBigInt("S",s);
		state = WAITING_FOR_REQ1;
		// see if we can find req1
		findReq1();
	}
	
	void EncryptedServerAuthenticate::findReq1()
	{
		if (buf_size < 116) // safety check
			return;
		
	//	Out() << "Find Req1" << endl;
		Uint8 tmp[100];
		memcpy(tmp,"req1",4);
		s.toBuffer(tmp + 4,96);
		SHA1Hash req1 = SHA1Hash::generate(tmp,100);
		for (Uint32 i = 96;i < buf_size - 20;i++)
		{
			if (buf[i] == req1.getData()[0] && memcmp(buf+i,req1.getData(),20) == 0)
			{
				state = FOUND_REQ1;
				req1_off = i;
				calculateSKey();
				return;
			}
		}
		
		if (buf_size > 608)
		{
	//		Out(SYS_CON|LOG_DEBUG) << "Couldn't find req1" << endl;
			onFinish(false);
		}
	}
	
	void EncryptedServerAuthenticate::calculateSKey()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Calculate SKEY" << endl;
		// not enough data return
		if (req1_off + 40 > buf_size)
			return;
		
		Uint8 tmp[100];
		memcpy(tmp,"req3",4);
		s.toBuffer(tmp+4,96);
		SHA1Hash r3 = SHA1Hash::generate(tmp,100);
		SHA1Hash r(buf + req1_off + 20);
		
		// r = HASH('req2', SKEY) xor HASH('req3', S)
		SHA1Hash r2 = r ^ r3; // now calculate HASH('req2', SKEY)
		if (!server->findInfoHash(r2,info_hash))
		{
	//		Out(SYS_CON|LOG_DEBUG) << "Unknown info_hash" << endl;
			onFinish(false);
			return;
		}
		// we have found the info_hash, now process VC and the rest
		state = FOUND_INFO_HASH;
		processVC();
	}
	
	void EncryptedServerAuthenticate::processVC()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Process VC" << endl;
		if (!our_rc4)
		{
			// calculate the keys
			SHA1Hash enc = mse::EncryptionKey(false,s,info_hash);
			SHA1Hash dec = mse::EncryptionKey(true,s,info_hash);
			//Out() << "enc = " << enc.toString() << endl;
			//Out() << "dec = " << dec.toString() << endl;
			our_rc4 = new RC4Encryptor(dec,enc);
		}
		
		// if we do not have everything return
		if (buf_size < req1_off + 40 + 14)
			return;
		
		
		Uint32 off = req1_off + 40;
		// now decrypt the vc and crypto_provide and the length of pad_C
		our_rc4->decrypt(buf + off,14);
			
		// check the VC
		for (Uint32 i = 0;i < 8;i++)
		{
			if (buf[off + i])
			{
		//		Out(SYS_CON|LOG_DEBUG) << "Illegal VC" << endl;
				onFinish(false);
				return;
			}
		}
		// get crypto_provide and the length of pad_C
		crypto_provide = bt::ReadUint32(buf,off + 8);
		pad_C_len = bt::ReadUint16(buf,off + 12);
		if (pad_C_len > 512)
		{
			Out(SYS_CON|LOG_DEBUG) << "Illegal pad C length" << endl;
			onFinish(false);
			return;
		}
		
		// now we have crypto_provide we can send 
		// ENCRYPT(VC, crypto_select, len(padD), padD)
		Uint8 tmp[14];
		memset(tmp,0,14); // VC
		if (crypto_provide & 0x0000002) // RC4 
		{
			WriteUint32(tmp,8,0x0000002);
			crypto_select = 0x0000002;
		}
		else
		{
			WriteUint32(tmp,8,0x0000001);
			crypto_select = 0x0000001;
		}
		bt::WriteUint16(tmp,12,0); // no pad D
		
		sock->sendData(our_rc4->encrypt(tmp,14),14);
		
		// handle pad C
		if (buf_size < req1_off + 14 + pad_C_len)
		{
			// we do not have the full padC
			state = WAIT_FOR_PAD_C;
			return;
		}
		
		handlePadC();
	}
	
	void EncryptedServerAuthenticate::handlePadC()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Handle PAD C" << endl;
		// not enough data, so return, we need padC and the length of IA
		if (buf_size < req1_off + 54 + pad_C_len + 2)
			return;
		
		// we have decrypted everything up to pad_C_len
		Uint32 off = req1_off + 54;
		our_rc4->decrypt(buf + off,pad_C_len + 2);
		ia_len = bt::ReadUint16(buf,off + pad_C_len);
		if (buf_size < off + ia_len)
		{
			// we do not have the IA, so wait for it
			state = WAIT_FOR_IA;
			return;
		}
		handleIA();
	}
	
	void EncryptedServerAuthenticate::handleIA()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Handle IA" << endl;
		// not enough data, so return, we need padC and the length of IA
		if (buf_size < req1_off + 54 + pad_C_len + 2 + ia_len)
			return;
		
		// decrypt the initial argument
		if (ia_len > 0)
		{
			Uint32 off = req1_off + 54 + pad_C_len + 2;
			// reinsert everything so that the normal authentication can handle it
			sock->reinsert(buf + off,buf_size - off);
		}
		
		bool allow_unenc = Globals::instance().getServer().unencryptedConnectionsAllowed();
		
		if (crypto_select & 0x0000002)
		{
			sock->setRC4Encryptor(our_rc4);
			our_rc4 = 0;
		}
		else if (!allow_unenc && crypto_select & 0x00000001)
		{
			// if no encrypted connections 
			Out(SYS_CON|LOG_DEBUG) << "Unencrypted connections not allowed" << endl;
			onFinish(false);
			return;
		}
		else
		{
			delete our_rc4;
			our_rc4 = 0;
		}
		
		// hand it over to ServerAuthenticate
		state = NON_ENCRYPTED_HANDSHAKE;
		ServerAuthenticate::onReadyRead();
	}
	
	void EncryptedServerAuthenticate::onReadyRead()
	{
		if (!sock)
			return;
		
		Uint32 ba = sock->bytesAvailable();
		if (!ba)
		{
			onFinish(false);
			return;
		}
		
		// make sure we don't write past the end of the buffer
		if (buf_size + ba > MAX_SEA_BUF_SIZE)
			ba = MAX_SEA_BUF_SIZE - buf_size;
		
		switch (state)
		{
		case WAITING_FOR_YA:
			if (ba <= 68 && Globals::instance().getServer().unencryptedConnectionsAllowed())
			{
				// this is most likely an unencrypted handshake, so if we can find a peer manager 
				// for the info hash in it, add it to the list of potential peers of that peer manager
				// so it will be contacted later on
			/*	buf_size += sock->readData(buf + buf_size,ba);
				if (buf_size >= 48)
				{
					SHA1Hash rh(buf+28);
					PeerManager* pman = server->findPeerManager(rh);
					if (pman)
					{
						PotentialPeer pp;
						pp.ip = sock->getRemoteIPAddress();
						pp.port = sock->getRemotePort();
						pman->addPotentialPeer(pp);
					}
				}
				onFinish(false);
			*/
				Out(SYS_CON|LOG_DEBUG) << "Switching back to normal server authenticate" << endl;
				state = NON_ENCRYPTED_HANDSHAKE;
				ServerAuthenticate::onReadyRead();
			}
			else
			{	
				buf_size += sock->readData(buf + buf_size,ba);
				if (buf_size >= 96)
					handleYA();
			}
			break;
		case WAITING_FOR_REQ1:
			if (buf_size + ba > MAX_SEA_BUF_SIZE)
				ba = MAX_SEA_BUF_SIZE - buf_size;
			
			buf_size += sock->readData(buf + buf_size,ba);
			findReq1();	
			break;
		case FOUND_REQ1:
			if (buf_size + ba > MAX_SEA_BUF_SIZE)
				ba = MAX_SEA_BUF_SIZE - buf_size;
			
			buf_size += sock->readData(buf + buf_size,ba);
			calculateSKey();
			break;
		case FOUND_INFO_HASH:
			if (buf_size + ba > MAX_SEA_BUF_SIZE)
				ba = MAX_SEA_BUF_SIZE - buf_size;
			
			buf_size += sock->readData(buf + buf_size,ba);
			processVC();
			break;
		case WAIT_FOR_PAD_C:
			if (buf_size + ba > MAX_SEA_BUF_SIZE)
				ba = MAX_SEA_BUF_SIZE - buf_size;
			
			buf_size += sock->readData(buf + buf_size,ba);
			handlePadC();
			break;
		case WAIT_FOR_IA:
			if (buf_size + ba > MAX_SEA_BUF_SIZE)
				ba = MAX_SEA_BUF_SIZE - buf_size;
			
			buf_size += sock->readData(buf + buf_size,ba);
			handleIA();
			break;
		case NON_ENCRYPTED_HANDSHAKE:
			ServerAuthenticate::onReadyRead();
			break;
		}
	}


}
#include "encryptedserverauthenticate.moc"
