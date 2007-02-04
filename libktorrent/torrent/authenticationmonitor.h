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
#ifndef BTAUTHENTICATIONMONITOR_H
#define BTAUTHENTICATIONMONITOR_H

#include <set>

namespace bt
{
	class AuthenticateBase;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Monitors ongoing authentication attempts. This class is a singleton.
	*/
	class AuthenticationMonitor
	{
		std::set<AuthenticateBase*> auths;
		
		static AuthenticationMonitor self;
		
		AuthenticationMonitor();
	public:
		
		virtual ~AuthenticationMonitor();
		
		
		/**
		 * Add a new AuthenticateBase object.
		 * @param s 
		 */
		void add(AuthenticateBase* s);
		
		/**
		 * Remove an AuthenticateBase object
		 * @param s 
		 */
		void remove(AuthenticateBase* s);
		
		/**
		 * Check all AuthenticateBase objects.
		 */
		void update();
		
		/**
		 * Clear all AuthenticateBase objects, also delets them
		 */
		void clear();
		
		static AuthenticationMonitor & instance() {return self;}
	};

}

#endif
