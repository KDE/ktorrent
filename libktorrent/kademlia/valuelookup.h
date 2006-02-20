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
#ifndef DHTVALUELOOKUP_H
#define DHTVALUELOOKUP_H

#include "task.h"

namespace dht
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class ValueLookup : public Task
	{
	public:
		ValueLookup(const dht::Key & key,RPCServer* rpc,Node* node);
		virtual ~ValueLookup();

		virtual void callFinished(RPCCall* , MsgBase* rsp);
		virtual void callTimeout(RPCCall* );
		virtual void update();
		
		/// Get the found value, will be a null array if nothing is found
		const QByteArray & getValue() const;
		
		/// Override isFinshed to first check for data in value
		virtual bool isFinished() const {return value.size() > 0 || Task::isFinished();}

	private:
		dht::Key key;
		QByteArray value;
	};

}

#endif
