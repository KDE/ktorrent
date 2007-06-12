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
#ifndef MSEFUNCTIONS_H
#define MSEFUNCTIONS_H

namespace bt
{
	class SHA1Hash;
}

namespace mse
{
	class BigInt;
	
	void GeneratePublicPrivateKey(BigInt & pub,BigInt & priv);
	BigInt DHSecret(const BigInt & our_priv,const BigInt & peer_pub);
	bt::SHA1Hash EncryptionKey(bool a,const BigInt & s,const bt::SHA1Hash & skey);
	
	void DumpBigInt(const QString & name,const BigInt & bi);
}

#endif
