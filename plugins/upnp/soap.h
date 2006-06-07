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
#ifndef KTSOAP_H
#define KTSOAP_H

#include <qvaluelist.h>
#include <qstring.h>

namespace kt
{

	/**
	@author Joris Guisson
	*/
	class SOAP
	{
	public:
		
		/**
		 * Create a simple UPnP SOAP command without parameters.
		 * @param action The name of the action
		 * @param service The name of the service
		 * @return The command
		 */
		static QString createCommand(const QString & action,const QString & service);
		
		struct Arg
		{
			QString element;
			QString value;
		};

		/**
		 * Create a UPnP SOAP command with parameters.
		 * @param action The name of the action
		 * @param service The name of the service
		 * @param args Arguments for command
		 * @return The command
		 */
		static QString createCommand(const QString & action,const QString & service,const QValueList<Arg> & args);
	};

}

#endif
