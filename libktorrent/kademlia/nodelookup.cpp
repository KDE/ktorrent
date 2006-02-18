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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "nodelookup.h"
#include "rpcmsg.h"
#include "node.h"

namespace dht
{

	NodeLookup::NodeLookup(const dht::Key & key,RPCServer* rpc,Node* node) 
	: Task(rpc),node_id(key),node(node)
	{
	}


	NodeLookup::~NodeLookup()
	{}


	void NodeLookup::callFinished(RPCCall* c, MsgBase* rsp)
	{
	}
	
	void NodeLookup::callTimeout(RPCCall* c)
	{}
	
	void NodeLookup::update()
	{
		QValueList<KBucketEntry>::iterator i = todo.begin();
		while (i != todo.end() && canDoRequest())
		{
			const KBucketEntry & e = *i;
			// send a findNode to the node
			FindNodeReq fnr(node->getOurID(),node_id);
			fnr.setOrigin(e.getAddress());
			rpcCall(&fnr);
			// TODO remove entry from todo list
			i++;
		}
	}
}
