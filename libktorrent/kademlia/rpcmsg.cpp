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
#include <torrent/bnode.h>
#include "rpcmsg.h"
#include "dht.h"

using namespace bt;

namespace dht
{
	const QString TID = "t";
	const QString REQ = "q";
	const QString RSP = "r";
	const QString TYP = "y";
	const QString ARG = "a";
	const QString ERR = "e";
	
	MsgBase* MakeMsg(bt::BDictNode* dict);
	
	
	MsgBase* ParseReq(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue(REQ);
		BDictNode*	args = dict->getDict(ARG);
		if (!vn || !args || !args->getValue("id") || !args->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		Uint32 mtid = args->getValue(TID)->data().toInt();
		
		QString str = vn->data().toString();
		if (str == "ping")
		{	
			return new PingReq(mtid,id);
		}
		else if (str == "find_node")
		{
			if (!args->getValue("target"))
				return 0;
			else
				return new FindNodeReq(mtid,id,Key(args->getValue("target")->data().toByteArray()));
		}
		else if (str == "find_value")
		{
			if (!args->getValue("key"))
				return 0;
			else
				return new FindValueReq(mtid,id,Key(args->getValue("key")->data().toByteArray()));
		}
		else if (str == "store_value")
		{
			if (!args->getValue("key") || !args->getValue("value"))
				return 0;
			
			return new StoreValueReq(mtid,id,
									 Key(args->getValue("key")->data().toByteArray()),
									 args->getValue("value")->data().toByteArray());
		}
		else if (str == "store_values")
		{
			if (!args->getValue("key") || !args->getValue("values"))
				return 0;
			
			return new StoreValueReq(mtid,id,
									 Key(args->getValue("key")->data().toByteArray()),
									 args->getValue("values")->data().toByteArray());
		}
		
		return 0;
	}

	MsgBase* ParseRsp(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue(RSP);
		BDictNode*	args = dict->getDict(ARG);
		if (!vn || !args || !args->getValue("id") || !args->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		Uint32 mtid = args->getValue(TID)->data().toInt();
		QString str = vn->data().toString();
		if (str == "ping")
		{	
			return new PingRsp(mtid,id);
		}
		else if (str == "find_node")
		{
			if (!args->getValue("nodes"))
				return 0;
			else
				return new FindNodeRsp(mtid,id,args->getValue("nodes")->data().toByteArray());
		}
		else if (str == "find_value")
		{
			if (!args->getValue("values"))
				return 0;
			else
				return new FindValueRsp(mtid,id,args->getValue("values")->data().toByteArray());
		}
		else if (str == "store_value")
		{
			return new StoreValueRsp(mtid,id);
		}
		else if (str == "store_values")
		{
			return new StoreValueRsp(mtid,id);
		}
		
		return 0;
	}
	
	MsgBase* ParseErr(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue(RSP);
		BDictNode*	args = dict->getDict(ARG);
		if (!vn || !args || !args->getValue("id") || !args->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		Uint32 mtid = args->getValue(TID)->data().toInt();
		QString str = vn->data().toString();
		
		return new ErrMsg(mtid,id,str);
	}
	
	
	MsgBase* MakeRPCMsg(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue(TYP);
		if (!vn)
			return 0;
		
		if (vn->data().toString() == REQ)
		{
			return ParseReq(dict);
		}
		else if (vn->data().toString() == RSP)
		{
			return ParseRsp(dict);
		}
		else if (vn->data().toString() == ERR)
		{
			return ParseErr(dict);
		}
		
		return 0;
	}
	
	MsgBase::MsgBase(Uint32 mtid,Method m,Type type,const Key & id)
	: mtid(mtid),method(m),type(type),id(id) 
	{}
	
	MsgBase::~MsgBase()
	{}
	
	////////////////////////////////
	
	PingReq::PingReq(Uint32 mtid,const Key & id) : MsgBase(mtid,PING,REQ_MSG,id)
	{
	}
	
	PingReq::~PingReq()
	{}
		
	void PingReq::apply(DHT* dh_table)
	{
		dh_table->ping(this);
	}
	
	////////////////////////////////
	
	FindNodeReq::FindNodeReq(Uint32 mtid,const Key & id,const Key & target)
	: MsgBase(mtid,FIND_NODE,REQ_MSG,id),target(target)
	{}
	
	FindNodeReq::~FindNodeReq()
	{}
		
	void FindNodeReq::apply(DHT* dh_table)
	{
		dh_table->findNode(this);
	}
	
	////////////////////////////////
	
	FindValueReq::FindValueReq(Uint32 mtid,const Key & id,const Key & key)
	: MsgBase(mtid,FIND_VALUE,REQ_MSG,id),key(key)
	{}
	
	FindValueReq::~FindValueReq()
	{}
	
	void FindValueReq::apply(DHT* dh_table)
	{
		dh_table->findValue(this);
	}

	////////////////////////////////
	StoreValueReq::StoreValueReq(Uint32 mtid,const Key & id,const Key & key,const QByteArray & ba)
	: MsgBase(mtid,STORE_VALUE,REQ_MSG,id),key(key),data(ba)
	{}
	
	StoreValueReq::~StoreValueReq()
	{}

	void StoreValueReq::apply(DHT* dh_table)
	{
		dh_table->storeValue(this);
	}
	
	////////////////////////////////
	
	PingRsp::PingRsp(Uint32 mtid,const Key & id)
	: MsgBase(mtid,PING,RSP_MSG,id)
	{}
	
	PingRsp::~PingRsp() {}
		
	void PingRsp::apply(DHT* dh_table) 
	{
		dh_table->ping(this);
	}
	
	////////////////////////////////
	
	FindNodeRsp::FindNodeRsp(Uint32 mtid,const Key & id,const QByteArray & nodes)
	: MsgBase(mtid,FIND_NODE,RSP_MSG,id),nodes(nodes)
	{}
	
	FindNodeRsp::~FindNodeRsp() {}
		
	void FindNodeRsp::apply(DHT* dh_table) 
	{
		dh_table->findNode(this);
	}
	
	////////////////////////////////
	
	FindValueRsp::FindValueRsp(Uint32 mtid,const Key & id,const QByteArray & values) 
	: MsgBase(mtid,FIND_VALUE,RSP_MSG,id),values(values)
	{}
	
	FindValueRsp::~FindValueRsp() {}
	
	void FindValueRsp::apply(DHT* dh_table) 
	{
		dh_table->findValue(this);
	}
	
	////////////////////////////////
	
	StoreValueRsp::StoreValueRsp(Uint32 mtid,const Key & id) 
	: MsgBase(mtid,STORE_VALUE,RSP_MSG,id) 
	{}
	
	StoreValueRsp::~StoreValueRsp() {}
		
	void StoreValueRsp::apply(DHT* dh_table) 
	{
		dh_table->storeValue(this);
	}
	
	////////////////////////////////
	
	ErrMsg::ErrMsg(Uint32 mtid,const Key & id,const QString & msg)
	: MsgBase(mtid,NONE,ERR_MSG,id),msg(msg)
	{}
	
	ErrMsg::~ErrMsg()
	{}
		
	void ErrMsg::apply(DHT* dh_table)
	{
		dh_table->error(this);
	}
}
