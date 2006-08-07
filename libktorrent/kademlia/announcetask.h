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
#ifndef DHTANNOUNCETASK_H
#define DHTANNOUNCETASK_H

#include <task.h>
#include "kbucket.h"

namespace dht
{
	class Database;
	
	class KBucketEntryAndToken : public KBucketEntry
	{
		Key token;
	public:
		KBucketEntryAndToken() {}
		KBucketEntryAndToken(const KBucketEntry & e,const Key & token)
			: KBucketEntry(e),token(token) {}
		virtual ~KBucketEntryAndToken() {}
		
		const Key & getToken() const {return token;}
	};

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class AnnounceTask : public Task
	{
	public:
		AnnounceTask(Database* db,RPCServer* rpc, Node* node,const dht::Key & info_hash,bt::Uint16 port);
		virtual ~AnnounceTask();

		virtual void callFinished(RPCCall* c, MsgBase* rsp);
		virtual void callTimeout(RPCCall* c);
		virtual void update();
		
		/**
		 * Take one item from the returned values.
		 * Returns false if there is no item to take.
		 * @param item The item
		 * @return false if no item to take, true else
		 */
		bool takeItem(DBItem & item);
	private:
		dht::Key info_hash;
		bt::Uint16 port;
		QValueList<KBucketEntryAndToken> answered; // nodes which have answered with values
		QValueList<KBucketEntry> answered_visited; // nodes which have answered with values which have been visited
		Database* db;
		DBItemList returned_items;
		
	};

}

#endif
