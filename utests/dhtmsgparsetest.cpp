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
#include <torrent/bdecoder.h>
#include <torrent/bnode.h>
#include <kademlia/rpcmsg.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "dhtmsgparsetest.h"

using namespace dht;
using namespace bt;

namespace utest
{
	

	DHTMsgParseTest::DHTMsgParseTest() : UnitTest("DHTMsgParseTest")
	{}


	DHTMsgParseTest::~DHTMsgParseTest()
	{}

	bool DHTMsgParseTest::doTest(const QString & data,int method)
	{
		QByteArray bdata(data.length());
		
		for (int i = 0;i < data.length();i++)
		{
			bdata[i] = data[i];
		}
		
		BDecoder bdec(bdata,false);
		
		BNode* n = bdec.decode();
		if (n->getType() != BNode::DICT)
		{
			delete n;
			Out() << "Packet does not contain a dictionary" << endl;
			return false;
		}
		
		MsgBase* msg = MakeRPCMsgTest((BDictNode*)n,(dht::Method)method);
		if (!msg)
		{
			delete n;
			Out() << "Error parsing message : " << endl;
			return false;
		}
		delete msg;
		delete n;
		return true;
	}
	
	

	bool DHTMsgParseTest::doTest()
	{	

		QString test_str[] = {
			"d1:rd2:id20:####################5:token20:####################6:valuesl6:######6:######6:######6:######6:######6:######6:######6:######ee1:t1:#1:y1:re",
			
			"d1:ad2:id20:####################9:info_hash20:####################e1:q9:get_peers1:t1:#1:y1:qe",
			
			"d1:rd2:id20:####################5:nodes208:################################################################################################################################################################################################################5:token20:####################e1:t1:#1:y1:re",
			
			QString::null
		};
		
		int types[] = {dht::GET_PEERS,dht::NONE,dht::GET_PEERS};
		
		int i = 0; 
		while (!test_str[i].isNull())
		{
			// read and decode the packet
			if (!doTest(test_str[i],types[i]))
			{
				Out() << "Testing packet " << i <<  " : Failed" << endl;
				return false;
			}
			else
			{
				Out() << "Testing packet " << i <<  " : OK" << endl;
			}
			i++;
		}
		
		return true;
	}

}
