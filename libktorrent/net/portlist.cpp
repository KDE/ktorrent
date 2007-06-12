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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "portlist.h"

namespace net
{
	Port::Port() : number(0),proto(TCP),forward(false)
	{
	}
	
	Port::Port(bt::Uint16 number,Protocol proto,bool forward)
	: number(number),proto(proto),forward(forward)
	{
	}
	
	Port::Port(const Port & p) : number(p.number),proto(p.proto),forward(p.forward)
	{
	}
	
	bool Port::operator == (const Port & p) const
	{
		return number == p.number && proto == p.proto;
	}

	PortList::PortList() : lst(0)
	{}


	PortList::~PortList()
	{}

		
	void PortList::addNewPort(bt::Uint16 number,Protocol proto,bool forward)
	{
		Port p = Port(number,proto,forward);
		append(p);
		if (lst)
			lst->portAdded(p);
	}
		
		
	void PortList::removePort(bt::Uint16 number,Protocol proto)
	{
		PortList::iterator itr = find(Port(number,proto,false));
		if (itr == end())
			return;
		
		if (lst)
			lst->portRemoved(*itr);
		
		erase(itr);
	}
		
	

}
