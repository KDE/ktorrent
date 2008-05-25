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
#include <time.h>
#include <stdlib.h>
#include <qcstring.h>
#include <util/constants.h>
#include "key.h"

using namespace bt;

namespace dht
{

	Key::Key()
	{}

	Key::Key(const bt::SHA1Hash & k) : bt::SHA1Hash(k)
	{
	}
	
	Key::Key(const Uint8* d) : bt::SHA1Hash(d)
	{
	}
	
	Key::Key(const QByteArray & ba)
	{
		for (Uint32 i = 0;i < 20 && i < ba.size();i++)
			hash[i] = ba[i];
	}

	Key::~Key()
	{}

	bool Key::operator == (const Key & other) const
	{
		return bt::SHA1Hash::operator ==(other);
	}
	
	bool Key::operator != (const Key & other) const
	{
		return !operator == (other);
	}
	
	bool Key::operator < (const Key & other) const
	{
		for (int i = 0;i < 20;i++)
		{
			if (hash[i] < other.hash[i])
				return true;
			else if (hash[i] > other.hash[i])
				return false;
		}
		return false;
	}
	
	bool Key::operator <= (const Key & other) const
	{
		return operator < (other) || operator == (other);
	}
	
	bool Key::operator > (const Key & other) const
	{
		for (int i = 0;i < 20;i++)
		{
			if (hash[i] < other.hash[i])
				return false;
			else if (hash[i] > other.hash[i])
				return true;
		}
		return false;
	}
	
	bool Key::operator >= (const Key & other) const
	{
		return operator > (other) || operator == (other);
	}

	Key Key::distance(const Key & a,const Key & b)
	{
		return a ^ b;
	}
	
	Key Key::random()
	{
		srand(time(0));
		Key k;
		for (int i = 0;i < 20;i++)
		{
			k.hash[i] = (Uint8)rand() % 0xFF;
		}
		return k;
	}
}
