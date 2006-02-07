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
#include "node.h"
#include "rpcmsg.h"


namespace dht
{

	Node::Node()
	{
		our_id = dht::Key::random();
		for (int i = 0;i < 160;i++)
			bucket[i] = 0;
	}


	Node::~Node()
	{
		for (int i = 0;i < 160;i++)
			delete bucket[i];
	}
	
	/*
	void Node::recieved(const RPCMsg & msg)
	{
		// get the id of the sender and calculate the distance between our ID 
		// and the sender's id
		dht::Key id = msg.getID();
		dht::Key dst = dht::Key::distance(our_id,id);
	}
	*/
}

#include "node.moc"
