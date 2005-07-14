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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "key.h"

namespace dht
{

	Key::Key()
	{}

	Key::Key(const Key & k) : bt::SHA1Hash(k)
	{
	}

	Key::~Key()
	{}

	Key & Key::operator = (const Key & k)
	{
		bt::SHA1Hash::operator = (k);
		return *this;
	}

	bool Key::operator == (const Key & other)
	{
		return bt::SHA1Hash::operator ==(other);
	}
	
	bool Key::operator != (const Key & other)
	{
		return !operator == (other);
	}
	
	bool Key::operator < (const Key & other)
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
	
	bool Key::operator <= (const Key & other)
	{
		return operator < (other) || operator == (other);
	}
	
	bool Key::operator > (const Key & other)
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
	
	bool Key::operator >= (const Key & other)
	{
		return operator > (other) || operator == (other);
	}

	Key Key::distance(const Key & a,const Key & b)
	{
		Key k;
		for (int i = 0;i < 20;i++)
		{
			k.hash[i] = a.hash[i] ^ b.hash[i];
		}
		return k;
	}
}
