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
#include <libutil/log.h>
#include "bnode.h"
#include "globals.h"

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
			Out() << "Value = " << v.toInt() << endl;
		else
			Out() << "Value = " << v.toString() << endl;
	}
	
	////////////////////////////////////////////////

	BDictNode::BDictNode(Uint32 off) : BNode(DICT,off)
	{
		children.setAutoDelete(true);
	}
	
	BDictNode::~BDictNode()
	{
	}
	
	void BDictNode::insert(const QString & key,BNode* node)
	{
		children.insert(key,node);
	}
	
	BNode* BDictNode::getData(const QString & key)
	{
		return children.find(key);
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
		Out() << "DICT" << endl;
		QDictIterator<BNode> it( children );
		for( ; it.current(); ++it )
		{
			Out() << it.currentKey() << ": " << endl;
			it.current()->printDebugInfo();
		}
		Out() << "END" << endl;
	}
	
	////////////////////////////////////////////////

	BListNode::BListNode(Uint32 off) : BNode(LIST,off)
	{
		children.setAutoDelete(true);
	}
	
	
	BListNode::~BListNode()
	{}
	
	
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
		Out() << "LIST " <<  children.count() << endl;
		for (Uint32 i = 0;i < children.count();i++)
		{
			BNode* n = children.at(i);
			n->printDebugInfo();
		}
		Out() << "END" << endl;
	}
}

