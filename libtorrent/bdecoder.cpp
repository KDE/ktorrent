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
#include "log.h"
#include "bdecoder.h"
#include "bnode.h"
#include "error.h"


namespace bt
{

	BDecoder::BDecoder(const QByteArray & data) : data(data),pos(0)
	{
	}


	BDecoder::~BDecoder()
	{}

	BNode* BDecoder::decode()
	{
		if (pos >= data.size())
			return 0;
	//	Out() << data[pos] << endl;
		if (data[pos] == 'd')
		{
			return parseDict();
		}
		else if (data[pos] == 'l')
		{
			return parseList();
		}
		else if (data[pos] == 'i')
		{
			return parseInt();
		}
		else if (data[pos] >= '0' && data[pos] <= '9')
		{
			return parseString();
		}
		else
		{
			throw Error(QString("Illegal token : %1").arg(data[pos]));
		}
	}

	BDictNode* BDecoder::parseDict()
	{
		Uint32 off = pos;
		// we're now entering a dictionary
		BDictNode* curr = new BDictNode(off);
		pos++;
	//	Out() << "DICT" << endl;
		try
		{
			while (data[pos] != 'e' && pos < data.size())
			{
			//	Out() << "Key : " << endl;
				BValueNode* k = dynamic_cast<BValueNode*>(decode());
				if (!k || k->data().getType() != Value::STRING)
					throw Error("Decode error");

				QString key = k->data().toString();
				delete k;

			//	Out() << "Data : " << endl;
				BNode* data = decode();
				curr->insert(key,data);
			}
			pos++;
		}
		catch (...)
		{
			delete curr;
			throw;
		}
	//	Out() << "END" << endl;
		curr->setLength(pos - off);
		return curr;
	}

	BListNode* BDecoder::parseList()
	{
		Uint32 off = pos;
	//	Out() << "LIST" << endl;
		BListNode* curr = new BListNode(off);
		pos++;
		try
		{
			while (data[pos] != 'e' && pos < data.size())
			{
				BNode* n = decode();
				curr->append(n);
			}
			pos++;
		}
		catch (...)
		{
			delete curr;
			throw;
		}
	//	Out() << "END" << endl;
		curr->setLength(pos - off);
		return curr;
	}

	BValueNode* BDecoder::parseInt()
	{
		Uint32 off = pos;
		pos++;
		QString n;
		// look for e and add everything between i and e to n
		while (pos < data.size() && data[pos] != 'e')
		{
			n += data[pos];
			pos++;
		}

		// check if we aren't at the end of the data
		if (pos >= data.size())
		{
			throw Error("Unexpected end of input");
		}

		// try to decode the int
		bool ok = true;
		int val = 0;
		val = n.toInt(&ok);
		if (!ok)
		{
			throw Error(QString("Cannot convert %1 to an int").arg(n));
		}
		pos++;
	//	Out() << "INT = " << val << endl;
		BValueNode* vn = new BValueNode(Value(val),off);
		vn->setLength(pos - off);
		return vn;
	}

	BValueNode* BDecoder::parseString()
	{
		Uint32 off = pos;
		// string are encoded 4:spam (length:string)

		// first get length by looking for the :
		QString n;
		while (pos < data.size() && data[pos] != ':')
		{
			n += data[pos];
			pos++;
		}
		// check if we aren't at the end of the data
		if (pos >= data.size())
		{
			throw Error("Unexpected end of input");
		}

		// try to decode length
		bool ok = true;
		int len = 0;
		len = n.toInt(&ok);
		if (!ok)
		{
			throw Error(QString("Cannot convert %1 to an int").arg(n));
		}
		// move pos to the first part of the string
		pos++;
		QByteArray arr(len);
		for (unsigned int i = pos;i < pos + len;i++)
			arr.at(i-pos) = data[i];
		pos += len;
		// read the string into n

		// pos should be positioned right after the string
		BValueNode* vn = new BValueNode(Value(arr),off);
		vn->setLength(pos - off);
	/*	if (arr.size() < 50)
			Out() << "STRING " << QString(arr) << endl;
		else
		Out() << "STRING " << "really long string" << endl;*/
		return vn;
	}
}

