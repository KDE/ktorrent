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
#include "bencoder.h"

namespace bt
{

	BEncoder::BEncoder()
	{}


	BEncoder::~BEncoder()
	{}


	QString BEncoder::encode(int val)
	{
		return QString("i%1e").arg(val);
	}
	
	QString BEncoder::encode(const QString & str)
	{
		return QString("%1:%2").arg(str.length()).arg(str);
	}
	
	QString BEncoder::encode(const Value & val)
	{
		switch (val.getType())
		{
			case Value::INT : 
				return encode(val.toInt());
			case Value::STRING :
				return encode(val.toString());
		}
		return QString::null;
	}
	
	QString BEncoder::encode(const QValueList<Value> & vl)
	{
		QString res = "l";
		for (QValueList<Value>::const_iterator i = vl.begin();i != vl.end();i++)
			res += encode(*i);
		res += "e";
		return res;
	}
	
	QString BEncoder::encode(const QMap<QString,Value> & vm)
	{
		QString res = "d";
		for (QMap<QString,Value>::const_iterator i = vm.begin();i != vm.end();i++)
			res += encode(i.key()) + encode(i.data());
		res += "e";
		return res;
	}
}
;
