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
#include <qtextcodec.h>
#include "value.h"

namespace bt
{

	Value::Value() : type(INT),ival(0)
	{}

	Value::Value(int val) : type(INT),ival(val)
	{}
	
	Value::Value(const QByteArray & val) : type(STRING),ival(0),strval(val)
	{}
	
	Value::Value(const Value & val) : type(val.type),ival(val.ival),strval(val.strval)
	{}

	Value::~Value()
	{}


	QString Value::toString(const QString & encoding) const
	{
		if (encoding.isNull() || encoding.isEmpty())
			return toString();

		QTextCodec* tc = QTextCodec::codecForName(encoding.ascii());
		if (!tc)
			return toString();

		return tc->toUnicode(strval);
	}
	

	Value & Value::operator = (const Value & val)
	{
		type = val.type;
		ival = val.ival;
		strval = val.strval;
		return *this;
	}
	
	Value & Value::operator = (int val)
	{
		type = INT;
		ival = val;
		return *this;
	}
	
	Value & Value::operator = (const QByteArray & val)
	{
		type = STRING;
		strval = val;
		return *this;
	}

}
