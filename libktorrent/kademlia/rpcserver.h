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
#ifndef DHTRPCSERVER_H
#define DHTRPCSERVER_H

#include <kdatagramsocket.h>
#include <util/constants.h>
#include <util/array.h>
#include <util/ptrmap.h>


using KNetwork::KDatagramSocket;
using bt::Uint16;
using bt::Uint8;

namespace dht
{
	class KBucketEntry;
	class RPCCall;

	/**
	 * @author Joris Guisson
	 *
	 * 
	 */
	class RPCServer : public QObject
	{
		Q_OBJECT
	public:
		RPCServer(Uint16 port,QObject *parent = 0);
		virtual ~RPCServer();

		RPCCall* ping(const KBucketEntry & to);
		RPCCall* findNode(const KBucketEntry & to,const Key & k);
		RPCCall* findValue(const KBucketEntry & to,const Key & k);
		RPCCall* store(const KBucketEntry & to,const Key & k,const bt::Array<Uint8> & data);

	private slots:
		void readPacket();

	private:
		KDatagramSocket* sock;
		bt::PtrMap<Key,RPCCall> active_calls;
	};

}

#endif
