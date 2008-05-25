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
#include <util/log.h>
#include <torrent/globals.h>
#include <util/sha1hash.h>
#include "functions.h"
#include "bigint.h"

using namespace bt;

namespace mse
{
	/*
	static const BigInt P = BigInt(
			"0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD"
			"129024E088A67CC74020BBEA63B139B22514A08798E3404"
			"DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C"
			"245E485B576625E7EC6F44C42E9A63A36210000000000090563");
	*/
	static const BigInt P = BigInt("0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563");
	
	void GeneratePublicPrivateKey(BigInt & priv,BigInt & pub)
	{
		BigInt G = BigInt("0x02");
		priv = BigInt::random();
		pub = BigInt::powerMod(G,priv,P);
	}

	BigInt DHSecret(const BigInt & our_priv,const BigInt & peer_pub)
	{
		return BigInt::powerMod(peer_pub,our_priv,P);
	}
	
	bt::SHA1Hash EncryptionKey(bool a,const BigInt & s,const bt::SHA1Hash & skey)
	{
		Uint8 buf[120];
		memcpy(buf,"key",3);
		buf[3] = (Uint8)(a ? 'A' : 'B');
		s.toBuffer(buf + 4,96);
		memcpy(buf + 100,skey.getData(),20);
		return bt::SHA1Hash::generate(buf,120);
	}
	
	void DumpBigInt(const QString & name,const BigInt & bi)
	{
		static Uint8 buf[512];
		Uint32 nb = bi.toBuffer(buf,512);
		bt::Log & lg = Out();
		lg << name << " (" << nb << ") = ";
		for (Uint32 i = 0;i < nb;i++)
		{
			lg << QString("0x%1 ").arg(buf[i],0,16);
		}
		lg << endl;
	}

}
