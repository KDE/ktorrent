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
#include "bnode.h"
#include <util/log.h>

namespace bt
{

	BNode::BNode(Type type,Uint32 off) : type(type),off(off),len(0)
	{
	}


	BNode::~BNode()
	{}

	////////////////////////////////////////////////

	BValueNode::BValueNode(const Value & v,Uint32 off) : BNode(VALUE,off),v(v)
	{}
	
	BValueNode::~BValueNode()
	{}
	
	void BValueNode::printDebugInfo()
	{
		if (v.getType() == Value::INT)
			Out(SYS_GEN|LOG_DEBUG) << "Value = " << v.toInt() << endl;
		else
			Out(SYS_GEN|LOG_DEBUG) << "Value = " << v.toString() << endl;
	}
	
	////////////////////////////////////////////////

	BDictNode::BDictNode(Uint32 off) : BNode(DICT,off)
	{
	}
	
	BDictNode::~BDictNode()
	{
		QList<DictEntry>::iterator i = children.begin();
		while (i != children.end())
		{
			DictEntry & e = *i;
			delete e.node;
			i++;
		}
	}
	
	QStringList BDictNode::keys() const
	{
		QStringList ret;
		QList<DictEntry>::const_iterator i = children.begin();
		while (i != children.end())
		{
			const DictEntry & e = *i;
			ret << QString(e.key);
			i++;
		}
		
		return ret;
	}
	
	void BDictNode::insert(const QByteArray & key,BNode* node)
	{
		DictEntry entry;
		entry.key = key;
		entry.node = node;
		children.append(entry);
	}
	
	BNode* BDictNode::getData(const QString & key)
	{
		QList<DictEntry>::iterator i = children.begin();
		while (i != children.end())
		{
			DictEntry & e = *i;
			if (QString(e.key) == key)
				return e.node;
			i++;
		}
		return 0;
	}
	
	BDictNode* BDictNode::getDict(const QByteArray & key)
	{
		QList<DictEntry>::iterator i = children.begin();
		while (i != children.end())
		{
			DictEntry & e = *i;
			if (e.key == key)
				return dynamic_cast<BDictNode*>(e.node);
			i++;
		}
		return 0;
	}

	BListNode* BDictNode::getList(const QString & key)
	{
		BNode* n = getData(key);
		return dynamic_cast<BListNode*>(n);
	}
	
	BDictNode* BDictNode::getDict(const QString & key)
	{
		BNode* n = getData(key);
		return dynamic_cast<BDictNode*>(n);
	}
	
	BValueNode* BDictNode::getValue(const QString & key)
	{
		BNode* n = getData(key);
		return dynamic_cast<BValueNode*>(n);
	}
	
	void BDictNode::printDebugInfo()
	{
		Out(SYS_GEN|LOG_DEBUG) << "DICT" << endl;
		QList<DictEntry>::iterator i = children.begin();
		while (i != children.end())
		{
			DictEntry & e = *i;
			Out(SYS_GEN|LOG_DEBUG) << QString(e.key) << ": " << endl;
			e.node->printDebugInfo();
			i++;
		}
		Out(SYS_GEN|LOG_DEBUG) << "END" << endl;
	}
	
	////////////////////////////////////////////////

	BListNode::BListNode(Uint32 off) : BNode(LIST,off)
	{
	}
	
	
	BListNode::~BListNode()
	{
		for (int i = 0;i < children.count();i++)
		{
			BNode* n = children.at(i);
			delete n;
		}
	}
	
	
	void BListNode::append(BNode* node)
	{
		children.append(node);
	}

	BListNode* BListNode::getList(Uint32 idx)
	{
		return dynamic_cast<BListNode*>(getChild(idx));
	}

	BDictNode* BListNode::getDict(Uint32 idx)
	{
		return dynamic_cast<BDictNode*>(getChild(idx));
	}

	BValueNode* BListNode::getValue(Uint32 idx)
	{
		return dynamic_cast<BValueNode*>(getChild(idx));
	}
	
	void BListNode::printDebugInfo()
	{
		Out(SYS_GEN|LOG_DEBUG) << "LIST " <<  children.count() << endl;
		for (int i = 0;i < children.count();i++)
		{
			BNode* n = children.at(i);
			n->printDebugInfo();
		}
		Out(SYS_GEN|LOG_DEBUG) << "END" << endl;
	}
}

