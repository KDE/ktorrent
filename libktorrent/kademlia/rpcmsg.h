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
#ifndef DHTRPCMSG_H
#define DHTRPCMSG_H

#include <ksocketaddress.h>
#include <util/constants.h>
#include "key.h"
#include "database.h"

namespace bt
{
	class BDictNode;
}

using bt::Uint8;
using bt::Uint32;

namespace dht
{
	class DHT;
	class RPCServer;
	
	enum Type
	{
		REQ_MSG,
		RSP_MSG,
		ERR_MSG,
		INVALID
	};
		
	enum Method
	{
		PING,
		FIND_NODE,
		FIND_VALUE,
		STORE_VALUE,
		STORE_VALUES,
		GET_PEERS,
		ANNOUNCE_PEER,
		NONE
	};
	
	const QString TID = "t";
	const QString REQ = "q";
	const QString RSP = "r";
	const QString TYP = "y";
	const QString ARG = "a";
	const QString ERR = "e";
	
	/**
	 * Base class for all RPC messages.
	*/
	class MsgBase 
	{
	public:
		MsgBase(Uint8 mtid,Method m,Type type,const Key & id);
		virtual ~MsgBase();
			
		
		/**
		 * When this message arrives this function will be called upon the DHT.
		 * The message should then call the appropriate DHT function (double dispatch)
		 * @param dh_table Pointer to DHT
		 */
		virtual void apply(DHT* dh_table) = 0;
		
		/**
		 * Print the message for debugging purposes.
		 */
		virtual void print() = 0;
		
		/**
		 * BEncode the message.
		 * @param arr Data array
		 */
		virtual void encode(QByteArray & arr) = 0;
		
		/// Set the origin (i.e. where the message came from)
		void setOrigin(const KNetwork::KSocketAddress & o) {origin = o;}
		
		/// Get the origin
		const KNetwork::KInetSocketAddress & getOrigin() const {return origin;}
		
		/// Get the MTID
		Uint8 getMTID() const {return mtid;}
		
		/// Set the MTID
		void setMTID(Uint8 m) {mtid = m;}
		
		/// Get the id of the sender
		const Key & getID() const {return id;}
		
		/// Get the type of the message
		Type getType() const {return type;}
		
		/// Get the message it's method
		Method getMethod() const {return method;}
		
	protected:
		Uint8 mtid;
		Method method;
		Type type;
		Key id;
		KNetwork::KInetSocketAddress origin;
	};
	
	/**
	 * Creates a message out of a BDictNode.
	 * @param dict The BDictNode
	 * @param srv The RPCServer
	 * @return A newly created message or 0 upon error
	 */
	MsgBase* MakeRPCMsg(bt::BDictNode* dict,RPCServer* srv);
	
	MsgBase* MakeRPCMsgTest(bt::BDictNode* dict,dht::Method req_method);
	
	class ErrMsg : public MsgBase
	{
	public:
		ErrMsg(Uint8 mtid,const Key & id,const QString & msg);
		virtual ~ErrMsg();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	private:
		QString msg;
	};
	
	class PingReq : public MsgBase
	{
	public:
		PingReq(const Key & id);
		virtual ~PingReq();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	};
	
	class FindNodeReq : public MsgBase
	{
	public:
		FindNodeReq(const Key & id,const Key & target);
		virtual ~FindNodeReq();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const Key & getTarget() const {return target;}
		
	private:
		Key target;
	};
	
	class FindValueReq : public MsgBase
	{
	public:
		FindValueReq(const Key & id,const Key & key);
		virtual ~FindValueReq();
	
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const Key & getKey() const {return key;}
		
	private:
		Key key;
	};

	
	class StoreValueReq : public MsgBase
	{
	public:
		StoreValueReq(const Key & id,const Key & key,const QByteArray & ba);
		virtual ~StoreValueReq();

		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const Key & getKey() const {return key;}
		const QByteArray & getData() const {return data;}
	private:
		Key key;
		QByteArray data;
	};
	
	class GetPeersReq : public MsgBase
	{
	public:
		GetPeersReq(const Key & id,const Key & info_hash);
		virtual ~GetPeersReq();
		
		const Key & getInfoHash() const {return info_hash;}
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	protected:
		Key info_hash;
	};
	
	class AnnounceReq : public GetPeersReq
	{
	public:
		AnnounceReq(const Key & id,const Key & info_hash,bt::Uint16 port,const Key & token);
		virtual ~AnnounceReq();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const Key & getToken() const {return token;}
		bt::Uint16 getPort() const {return port;}
	private:
		bt::Uint16 port;
		Key token;
	};
	
	class PingRsp : public MsgBase
	{
	public:
		PingRsp(Uint8 mtid,const Key & id);
		virtual ~PingRsp();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	};
	
	

	class FindNodeRsp : public MsgBase
	{
	public:
		FindNodeRsp(Uint8 mtid,const Key & id,const QByteArray & nodes);
		virtual ~FindNodeRsp();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const QByteArray & getNodes() const {return nodes;}
	protected:
		QByteArray nodes;
	};
	
	class GetPeersRsp : public MsgBase
	{
	public:
		GetPeersRsp(Uint8 mtid,const Key & id,const QByteArray & data,const Key & token);
		GetPeersRsp(Uint8 mtid,const Key & id,const DBItemList & values,const Key & token);
		virtual ~GetPeersRsp();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const QByteArray & getData() const {return data;}
		const DBItemList & getItemList() const {return items;}
		const Key & getToken() const {return token;}
		bool containsNodes() const {return data.size() > 0;}
		bool containsValues() const {return data.size() == 0;}
	private:
		Key token;
		QByteArray data;
		DBItemList items;
	};
	
	
	class FindValueRsp : public MsgBase
	{
	public:
		FindValueRsp(Uint8 mtid,const Key & id,const QByteArray & values);
		virtual ~FindValueRsp();
	
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
		
		const QByteArray & getValue() const {return values;}
	protected:
		QByteArray values;
	};
	
	class StoreValueRsp : public MsgBase
	{
	public:
		StoreValueRsp(Uint8 mtid,const Key & id);
		virtual ~StoreValueRsp();
		
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	};
	
	class AnnounceRsp : public MsgBase
	{
	public:
		AnnounceRsp(Uint8 mtid,const Key & id);
		virtual ~AnnounceRsp();
	
		virtual void apply(DHT* dh_table);
		virtual void print();
		virtual void encode(QByteArray & arr);
	};
	
	
}

#endif
