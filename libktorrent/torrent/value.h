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
#ifndef BTVALUE_H
#define BTVALUE_H

#include <qstring.h>
#include <util/constants.h>

namespace bt
{

	/**
	@author Joris Guisson
	*/
	class Value
	{
	public:
		enum Type
		{
			STRING,INT,INT64
		};
		
	
		Value();
		Value(int val);
		Value(Int64 val);
		Value(const QByteArray & val);
		Value(const Value & val);
		~Value();

		Value & operator = (const Value & val);
		Value & operator = (Int32 val);
		Value & operator = (Int64 val);
		Value & operator = (const QByteArray & val);
		
		Type getType() const {return type;}
		Int32 toInt() const {return ival;}
		Int64 toInt64() const {return big_ival;}
		QString toString() const {return QString(strval);}
		QString toString(const QString & encoding) const;
		QByteArray toByteArray() const {return strval;}
	private:
		Type type;
		Int32 ival;
		QByteArray strval;
		Int64 big_ival;
	};
}

#endif
