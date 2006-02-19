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
#ifndef DHTSTOREVALUE_H
#define DHTSTOREVALUE_H

#include "task.h"

namespace dht
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class StoreValue : public Task
	{
	public:
		StoreValue(RPCServer* rpc);
		virtual ~StoreValue();

		virtual void callFinished(RPCCall* c, MsgBase* rsp);
		virtual void callTimeout(RPCCall* c);
		virtual void update();

	};

}

#endif
