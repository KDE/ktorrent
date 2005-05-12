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
#ifndef BTSHA1HASH_H
#define BTSHA1HASH_H



namespace bt
{
	class Log;

	/**
	@author Joris Guisson
	*/
	class SHA1Hash
	{
		unsigned char hash[20];
	public:
		SHA1Hash();
		SHA1Hash(const SHA1Hash & other);
		SHA1Hash(const unsigned char* h);
		~SHA1Hash();

		SHA1Hash & operator = (const SHA1Hash & other);

		bool operator == (const SHA1Hash & other) const;
		bool operator != (const SHA1Hash & other) const {return !operator ==(other);}

		static SHA1Hash generate(unsigned char* data,unsigned int len);

		QString toString() const;
		QString toURLString() const;
		const unsigned char* getData() const {return hash;}

		friend Log & operator << (Log & out,const SHA1Hash & h);
	};

};

#endif
