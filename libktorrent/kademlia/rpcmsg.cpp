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
#include <util/log.h>
#include <torrent/bnode.h>
#include <torrent/globals.h>
#include <torrent/bencoder.h>
#include "rpcmsg.h"
#include "dht.h"

using namespace bt;

namespace dht
{

	
	MsgBase* MakeMsg(bt::BDictNode* dict);
	
	
	MsgBase* ParseReq(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue(REQ);
		BDictNode*	args = dict->getDict(ARG);
		if (!vn || !args)
			return 0;
		
		if (!args->getValue("id"))
			return 0;
		
		if (!dict->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		QString mt_id = dict->getValue(TID)->data().toString();
		Uint8 mtid = (char)mt_id.at(0).latin1();
		
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
		if (!vn || !args || !args->getValue("id") || !dict->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		QString mt_id = dict->getValue(TID)->data().toString();
		Uint8 mtid = (char)mt_id.at(0).latin1();
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
		if (!vn || !args || !args->getValue("id") || !dict->getValue(TID))
			return 0;
			
		Key id = Key(args->getValue("id")->data().toByteArray());
		QString mt_id = dict->getValue(TID)->data().toString();
		Uint8 mtid = (char)mt_id.at(0).latin1();
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
	
	MsgBase::MsgBase(Uint8 mtid,Method m,Type type,const Key & id)
	: mtid(mtid),method(m),type(type),id(id) 
	{}
	
	MsgBase::~MsgBase()
	{}
	
	////////////////////////////////
	
	PingReq::PingReq(Uint8 mtid,const Key & id) : MsgBase(mtid,PING,REQ_MSG,id)
	{
	}
	
	PingReq::~PingReq()
	{}
		
	void PingReq::apply(DHT* dh_table)
	{
		dh_table->ping(this);
	}
	
	void PingReq::print()
	{
		Out() << QString("REQ: %1 %2 : ping").arg(mtid).arg(id.toString()) << endl;
	}
	
	void PingReq::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(TYP); enc.write(REQ);
			enc.write(REQ); enc.write("ping");
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	FindNodeReq::FindNodeReq(Uint8 mtid,const Key & id,const Key & target)
	: MsgBase(mtid,FIND_NODE,REQ_MSG,id),target(target)
	{}
	
	FindNodeReq::~FindNodeReq()
	{}
		
	void FindNodeReq::apply(DHT* dh_table)
	{
		dh_table->findNode(this);
	}
	
	void FindNodeReq::print()
	{
		Out() << QString("REQ: %1 %2 : find_node %3")
				.arg(mtid).arg(id.toString()).arg(target.toString()) << endl;
	}
	
	void FindNodeReq::encode(QByteArray & arr)
	{}
	
	////////////////////////////////
	
	FindValueReq::FindValueReq(Uint8 mtid,const Key & id,const Key & key)
	: MsgBase(mtid,FIND_VALUE,REQ_MSG,id),key(key)
	{}
	
	FindValueReq::~FindValueReq()
	{}
	
	void FindValueReq::apply(DHT* dh_table)
	{
		dh_table->findValue(this);
	}
	
	void FindValueReq::print()
	{
		Out() << QString("REQ: %1 %2 : find_value %3")
				.arg(mtid).arg(id.toString()).arg(key.toString()) << endl;
	}
	
	void FindValueReq::encode(QByteArray & arr)
	{}

	////////////////////////////////
	StoreValueReq::StoreValueReq(Uint8 mtid,const Key & id,const Key & key,const QByteArray & ba)
	: MsgBase(mtid,STORE_VALUE,REQ_MSG,id),key(key),data(ba)
	{}
	
	StoreValueReq::~StoreValueReq()
	{}

	void StoreValueReq::apply(DHT* dh_table)
	{
		dh_table->storeValue(this);
	}
	
	void StoreValueReq::print()
	{
		Out() << QString("REQ: %1 %2 : store_value %3")
				.arg(mtid).arg(id.toString()).arg(id.toString()).arg(key.toString()) << endl;
	}
	
	void StoreValueReq::encode(QByteArray & arr)
	{}
	
	////////////////////////////////
	
	PingRsp::PingRsp(Uint8 mtid,const Key & id)
	: MsgBase(mtid,PING,RSP_MSG,id)
	{}
	
	PingRsp::~PingRsp() {}
		
	void PingRsp::apply(DHT* dh_table) 
	{
		dh_table->ping(this);
	}
	
	void PingRsp::print()
	{
		Out() << QString("RSP: %1 %2 : ping")
					.arg(mtid).arg(id.toString()) << endl;
	}
	
	void PingRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(TYP); enc.write(RSP);
			enc.write(RSP); enc.write("ping");
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	FindNodeRsp::FindNodeRsp(Uint8 mtid,const Key & id,const QByteArray & nodes)
	: MsgBase(mtid,FIND_NODE,RSP_MSG,id),nodes(nodes)
	{}
	
	FindNodeRsp::~FindNodeRsp() {}
		
	void FindNodeRsp::apply(DHT* dh_table) 
	{
		dh_table->findNode(this);
	}
	
	void FindNodeRsp::print()
	{
		Out() << QString("RSP: %1 %2 : find_node")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void FindNodeRsp::encode(QByteArray & arr)
	{}

	
	////////////////////////////////
	
	FindValueRsp::FindValueRsp(Uint8 mtid,const Key & id,const QByteArray & values) 
	: MsgBase(mtid,FIND_VALUE,RSP_MSG,id),values(values)
	{}
	
	FindValueRsp::~FindValueRsp() {}
	
	void FindValueRsp::apply(DHT* dh_table) 
	{
		dh_table->findValue(this);
	}
	
	void FindValueRsp::print()
	{
		Out() << QString("RSP: %1 %2 : find_value")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void FindValueRsp::encode(QByteArray & arr)
	{}
	
	////////////////////////////////
	
	StoreValueRsp::StoreValueRsp(Uint8 mtid,const Key & id) 
	: MsgBase(mtid,STORE_VALUE,RSP_MSG,id) 
	{}
	
	StoreValueRsp::~StoreValueRsp() {}
		
	void StoreValueRsp::apply(DHT* dh_table) 
	{
		dh_table->storeValue(this);
	}
	
	void StoreValueRsp::print()
	{
		Out() << QString("RSP: %1 %2 : store_value")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void StoreValueRsp::encode(QByteArray & arr)
	{}
	
	////////////////////////////////
	
	ErrMsg::ErrMsg(Uint8 mtid,const Key & id,const QString & msg)
	: MsgBase(mtid,NONE,ERR_MSG,id),msg(msg)
	{}
	
	ErrMsg::~ErrMsg()
	{}
		
	void ErrMsg::apply(DHT* dh_table)
	{
		dh_table->error(this);
	}
	
	void ErrMsg::print()
	{
		Out() << "ERR: " << mtid << " " << msg << endl;
	}
	
	void ErrMsg::encode(QByteArray & arr)
	{}
}
