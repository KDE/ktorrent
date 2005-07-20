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
#include "rpccall.h"

namespace dht
{
	const QString TID = "t";
	const QString REQ = "q";
	const QString RSP = "r";
	const QString TYP = "y";
	const QString ARG = "a";
	const QString ERR = "e";

	//msg = {TID : chr(self.mtid), TYP : REQ,  REQ : method, ARG : args}

	RPCCall::RPCCall(QObject *parent) : QObject(parent)
	{}


	RPCCall::~RPCCall()
	{}


}
#include "rpccall.moc"
