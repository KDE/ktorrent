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
#include "rpccall.h"
#include "rpcserver.h"
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
		Uint8 mtid = (Uint8)dict->getValue(TID)->data().toByteArray().at(0);
		MsgBase* msg = 0;
		
		QString str = vn->data().toString();
		if (str == "ping")
		{	
			msg = new PingReq(id);
		}
		else if (str == "find_node")
		{
			if (args->getValue("target"))
				msg = new FindNodeReq(id,Key(args->getValue("target")->data().toByteArray()));
		}
		else if (str == "find_value")
		{
			if (args->getValue("key"))
				msg = new FindValueReq(id,Key(args->getValue("key")->data().toByteArray()));
		}
		else if (str == "store_value")
		{
			if (args->getValue("key") && args->getValue("value"))
				msg = new StoreValueReq(id,
									 Key(args->getValue("key")->data().toByteArray()),
									 args->getValue("value")->data().toByteArray());
		}
		else if (str == "store_values")
		{
			if (args->getValue("key") && args->getValue("values"))
				msg = new StoreValueReq(id,
									 Key(args->getValue("key")->data().toByteArray()),
									 args->getValue("values")->data().toByteArray());
		}
		else if (str == "get_peers")
		{
			if (args->getValue("info_hash"))
				msg = new GetPeersReq(id,Key(args->getValue("info_hash")->data().toByteArray()));
		}
		else if (str == "announce_peer")
		{
			if (args->getValue("info_hash") && args->getValue("port") && args->getValue("token"))
			{
				msg = new AnnounceReq(id,
						Key(args->getValue("info_hash")->data().toByteArray()),
						args->getValue("port")->data().toInt(),
						Key(args->getValue("token")->data().toByteArray()));
			}
		}
		
		if (msg)
			msg->setMTID(mtid);
		
		return msg;
	}
	
	MsgBase* ParseRsp(bt::BDictNode* dict,dht::Method req_method,Uint8 mtid)
	{
		BDictNode* args = dict->getDict(RSP);
		if (!args || !args->getValue("id"))
			return 0;
		
		Key id = Key(args->getValue("id")->data().toByteArray());
		
		switch (req_method)
		{
			case PING : 
				return new PingRsp(mtid,id);
			case FIND_NODE :
				if (!args->getValue("nodes"))
					return 0;
				else
					return new FindNodeRsp(mtid,id,args->getValue("nodes")->data().toByteArray());
			case FIND_VALUE:
				if (!args->getValue("values"))
				{
					// FIND_VALUE will return a FIND_NODE when it does not know
					// the key
					if (!args->getValue("nodes"))
						return 0;
					else
						return new FindNodeRsp(mtid,id,args->getValue("nodes")->data().toByteArray());
				}
				else
					return new FindValueRsp(mtid,id,args->getValue("values")->data().toByteArray());
			case STORE_VALUE :
			case STORE_VALUES :
				return new StoreValueRsp(mtid,id);
			case GET_PEERS :
				if (args->getValue("token"))
				{
					Key token = args->getValue("token")->data().toByteArray();
					QByteArray data;
					BListNode* vals = args->getList("values");
					DBItemList dbl;
					if (vals)
					{
						for (Uint32 i = 0;i < vals->getNumChildren();i++)
						{
							BValueNode* vn = dynamic_cast<BValueNode*>(vals->getChild(i));
							if (!vn)
								continue;
							dbl.append(DBItem((Uint8*)vn->data().toByteArray().data()));
						}
						return new GetPeersRsp(mtid,id,dbl,token);
					}
					else if (args->getValue("nodes"))
					{
						data = args->getValue("nodes")->data().toByteArray();
						return new GetPeersRsp(mtid,id,data,token);
					}
					else
					{
						Out() << "No nodes or values in get_peers response" << endl;
						return 0;
					}
				}
				else
				{
					Out() << "No token in get_peers response" << endl;
				}
			case ANNOUNCE_PEER :
				return new AnnounceRsp(mtid,id);
			default:
				return 0;
		}
		return 0;
	}

	MsgBase* ParseRsp(bt::BDictNode* dict,RPCServer* srv)
	{
		BDictNode*	args = dict->getDict(RSP);
		if (!args || !dict->getValue(TID))
		{
			Out() << "ParseRsp : args || !args->getValue(id) || !dict->getValue(TID)" << endl;
			return 0;
		}
			
		
		Uint8 mtid = (Uint8)dict->getValue(TID)->data().toByteArray().at(0);
		// find the call
		const RPCCall* c = srv->findCall(mtid);
		if (!c)
		{
			Out() << "Cannot find RPC call" << endl;
			return 0;
		}
	
		return ParseRsp(dict,c->getMsgMethod(),mtid);
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
	
	
	MsgBase* MakeRPCMsg(bt::BDictNode* dict,RPCServer* srv)
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
			return ParseRsp(dict,srv);
		}
		else if (vn->data().toString() == ERR)
		{
			return ParseErr(dict);
		}
		
		return 0;
	}
	
	MsgBase* MakeRPCMsgTest(bt::BDictNode* dict,dht::Method req_method)
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
			return ParseRsp(dict,req_method,0);
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
	
	PingReq::PingReq(const Key & id) : MsgBase(0xFF,PING,REQ_MSG,id)
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
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(REQ); enc.write("ping");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	FindNodeReq::FindNodeReq(const Key & id,const Key & target)
	: MsgBase(0xFF,FIND_NODE,REQ_MSG,id),target(target)
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
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("target"); enc.write(target.getData(),20);
			}
			enc.end();
			enc.write(REQ); enc.write("find_node");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	FindValueReq::FindValueReq(const Key & id,const Key & key)
	: MsgBase(0xFF,FIND_VALUE,REQ_MSG,id),key(key)
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
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("key"); enc.write(key.getData(),20);
			}
			enc.end();
			enc.write(REQ); enc.write("find_value");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}

	////////////////////////////////
	StoreValueReq::StoreValueReq(const Key & id,const Key & key,const QByteArray & ba)
	: MsgBase(0xFF,STORE_VALUE,REQ_MSG,id),key(key),data(ba)
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
				.arg(mtid).arg(id.toString()).arg(key.toString()) << endl;
	}
	
	void StoreValueReq::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("key"); enc.write(key.getData(),20);
				enc.write("value"); enc.write(data);
			}
			enc.end();
			enc.write(REQ); enc.write("store_value");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}
	
	////////////////////////////////
	GetPeersReq::GetPeersReq(const Key & id,const Key & info_hash) 
		: MsgBase(0xFF,GET_PEERS,REQ_MSG,id),info_hash(info_hash)
	{}
	
	GetPeersReq::~GetPeersReq()
	{}
		
	void GetPeersReq::apply(DHT* dh_table)
	{
		dh_table->getPeers(this);
	}
	
	void GetPeersReq::print()
	{
		Out() << QString("REQ: %1 %2 : get_peers %3")
				.arg(mtid).arg(id.toString()).arg(info_hash.toString()) << endl;
	}
	
	void GetPeersReq::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("info_hash"); enc.write(info_hash.getData(),20);
			}
			enc.end();
			enc.write(REQ); enc.write("get_peers");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	AnnounceReq::AnnounceReq(const Key & id,const Key & info_hash,Uint16 port,const Key & token) 
	: GetPeersReq(id,info_hash),port(port),token(token)
	{
		method = dht::ANNOUNCE_PEER;
	}
	
	AnnounceReq::~AnnounceReq() {}
		
	void AnnounceReq::apply(DHT* dh_table)
	{
		dh_table->announce(this);
	}
	
	void AnnounceReq::print()
	{
		Out() << QString("REQ: %1 %2 : announce_peer %3 %4 %5")
				.arg(mtid).arg(id.toString()).arg(info_hash.toString())
				.arg(port).arg(token.toString()) << endl;
	}
	
	void AnnounceReq::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(ARG); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("info_hash"); enc.write(info_hash.getData(),20);
				enc.write("port"); enc.write((Uint32)port);
				enc.write("token"); enc.write(token.getData(),20);
			}
			enc.end();
			enc.write(REQ); enc.write("announce_peer");
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(REQ);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	PingRsp::PingRsp(Uint8 mtid,const Key & id)
	: MsgBase(mtid,PING,RSP_MSG,id)
	{}
	
	PingRsp::~PingRsp() {}
		
	void PingRsp::apply(DHT* dh_table) 
	{
		dh_table->response(this);
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
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
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
		dh_table->response(this);
	}
	
	void FindNodeRsp::print()
	{
		Out() << QString("RSP: %1 %2 : find_node")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void FindNodeRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("nodes"); enc.write(nodes);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	GetPeersRsp::GetPeersRsp(Uint8 mtid,const Key & id,const QByteArray & data,const Key & token)
	: MsgBase(mtid,dht::GET_PEERS,dht::RSP_MSG,id),token(token),data(data)
	{
		this->data.detach();
	}
	
	GetPeersRsp::GetPeersRsp(Uint8 mtid,const Key & id,const DBItemList & values,const Key & token)
	: MsgBase(mtid,dht::GET_PEERS,dht::RSP_MSG,id),token(token),items(values)
	{}
		
	GetPeersRsp::~GetPeersRsp()
	{}
		
	void GetPeersRsp::apply(DHT* dh_table) 
	{
		dh_table->response(this);
	}
	void GetPeersRsp::print()
	{
		Out() << QString("RSP: %1 %2 : get_peers(%3)")
				.arg(mtid).arg(id.toString()).arg(data.size() > 0 ? "nodes" : "values") << endl;
	}

	void GetPeersRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				if (data.size() > 0)
				{
					enc.write("nodes"); enc.write(data);
					enc.write("token"); enc.write(token.getData(),20);
				}
				else
				{
					enc.write("token"); enc.write(token.getData(),20);
					enc.write("values"); enc.beginList();
					DBItemList::iterator i = items.begin();
					while (i != items.end())
					{
						const DBItem & item = *i;
						enc.write(item.getData(),6);
						i++;
					}
					enc.end();
				}
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
		}
		enc.end();
	}

	
	////////////////////////////////
	
	FindValueRsp::FindValueRsp(Uint8 mtid,const Key & id,const QByteArray & values) 
	: MsgBase(mtid,FIND_VALUE,RSP_MSG,id),values(values)
	{}
	
	FindValueRsp::~FindValueRsp() {}
	
	void FindValueRsp::apply(DHT* dh_table) 
	{
		dh_table->response(this);
	}
	
	void FindValueRsp::print()
	{
		Out() << QString("RSP: %1 %2 : find_value")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void FindValueRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
				enc.write("values"); enc.write(values);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	StoreValueRsp::StoreValueRsp(Uint8 mtid,const Key & id) 
	: MsgBase(mtid,STORE_VALUE,RSP_MSG,id) 
	{}
	
	StoreValueRsp::~StoreValueRsp() {}
		
	void StoreValueRsp::apply(DHT* dh_table) 
	{
		dh_table->response(this);
	}
	
	void StoreValueRsp::print()
	{
		Out() << QString("RSP: %1 %2 : store_value")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void StoreValueRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
		}
		enc.end();
	}
	
	////////////////////////////////
	
	AnnounceRsp::AnnounceRsp(Uint8 mtid,const Key & id) : MsgBase(mtid,ANNOUNCE_PEER,RSP_MSG,id)
	{}
	
	AnnounceRsp::~AnnounceRsp(){}
	
	void AnnounceRsp::apply(DHT* dh_table)
	{
		dh_table->response(this);
	}
	
	void AnnounceRsp::print()
	{
		Out() << QString("RSP: %1 %2 : announce_peer")
				.arg(mtid).arg(id.toString()) << endl;
	}
	
	void AnnounceRsp::encode(QByteArray & arr)
	{
		BEncoder enc(new BEncoderBufferOutput(arr));
		enc.beginDict();
		{
			enc.write(RSP); enc.beginDict();
			{
				enc.write("id"); enc.write(id.getData(),20);
			}
			enc.end();
			enc.write(TID); enc.write(&mtid,1);
			enc.write(TYP); enc.write(RSP);
		}
		enc.end();
	}
	
	
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
