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
#ifndef NETPORTLIST_H
#define NETPORTLIST_H

#include <QList>
#include <util/constants.h>
#include <btcore_export.h>

namespace net
{
	enum Protocol
	{
		TCP,
		UDP
	};
	
	struct BTCORE_EXPORT Port
	{
		bt::Uint16 number;
		Protocol proto;
		bool forward;
		
		Port();
		Port(bt::Uint16 number,Protocol proto,bool forward);
		Port(const Port & p);
		
		bool operator == (const Port & p) const;
	};
	
	/**
	 * Listener class for the PortList. 
	 */
	class BTCORE_EXPORT PortListener
	{
	public:
		virtual ~PortListener() {}

		/**
		 * A port has been added.
		 * @param port The port
		 */
		virtual void portAdded(const Port & port) = 0;
		
		/**
		 * A port has been removed
		 * @param port The port
		 */
		virtual void portRemoved(const Port & port) = 0;
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * List of ports which are currently being used.
	 * 
	*/
	class BTCORE_EXPORT PortList : public QList<Port>
	{
		PortListener* lst;
	public:
		PortList();
		virtual ~PortList();

		/**
		 * When a port is in use, this function needs to be called.
		 * @param number Port number
		 * @param proto Protocol
		 * @param forward Whether or not it needs to be forwarded
		 */
		void addNewPort(bt::Uint16 number,Protocol proto,bool forward);
		
		/**
		 * Needs to be called when a port is not being using anymore.
		 * @param number Port number
		 * @param proto Protocol
		 */
		void removePort(bt::Uint16 number,Protocol proto);
		
		/**
		 * Set the port listener.
		 * @param pl Port listener
		 */
		void setListener(PortListener* pl) {lst = pl;}
	};

}

#ifdef Q_CC_MSVC
#include <QHash>
inline uint qHash(const net::Port & port) {return qHash((uint)port.number);}
#endif


#endif
