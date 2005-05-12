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
#ifndef BTBNODE_H
#define BTBNODE_H

#include <qptrlist.h>
#include <qdict.h>
#include "value.h"
#include "globals.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Base class for a node in a b-encoded piece of data
	 * 
	 * There are 3 possible pieces of data in b-encoded piece of data.
	 * This is the base class for all those 3 things.
	*/
	class BNode
	{
	public:
		enum Type
		{
			VALUE,DICT,LIST
		};
		
		/**
		 * Constructor, sets the Type, and the offset into 
		 * the data.
		 * @param type Type of node
		 * @param off The offset into the data
		 */
		BNode(Type type,Uint32 off);
		virtual ~BNode();
		
		Type getType() const {return type;}
		
		Uint32 getOffset() const {return off;}
		Uint32 getLength() const {return len;}
		void setLength(Uint32 l) {len = l;}
		
		virtual void printDebugInfo() = 0;
	private:
		Type type;
		Uint32 off,len;
	};
	
	class BValueNode : public BNode
	{
		Value v;
	public:
		BValueNode(const Value & v,Uint32 off);
		virtual ~BValueNode();
		
		const Value & data() const {return v;}
		void printDebugInfo();
	};

	class BDictNode : public BNode
	{
		QDict<BNode> children;
	public:
		BDictNode(Uint32 off);
		virtual ~BDictNode();
		
		void insert(const QString & key,BNode* node);
		BNode* getData(const QString & key);
		void printDebugInfo();
	};
	
	class BListNode : public BNode
	{
		QPtrList<BNode> children;
	public:
		BListNode(Uint32 off);
		virtual ~BListNode();

		void append(BNode* node);
		void printDebugInfo();
		Uint32 getNumChildren() const {return children.count();}
		BNode* getChild(Uint32 idx) {return children.at(idx);}
	};
};

#endif
