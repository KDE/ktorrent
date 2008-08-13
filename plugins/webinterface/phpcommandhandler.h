/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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
#ifndef KT_PHPCOMMANDHANDLER_HH
#define KT_PHPCOMMANDHANDLER_HH

#include <QMap>
#include <QString>

namespace kt
{
	class CoreInterface;
	class PhpCommand;
	
	
	/**
	 * Class which handles PHP commands provided by the URL arguments
	 */
	class PhpCommandHandler
	{
	public:
		PhpCommandHandler(CoreInterface *c);
		virtual ~PhpCommandHandler();
		
		/**
		 * Execute the URL arguments provided to the PHP script
		 * @param url The request url, used keys are erased from it
		 * @param shutdown Set to true if we need to shutdown KT
		 */
		bool exec(KUrl & url,bool & shutdown);
	private:
		CoreInterface* core;
	};
}

#endif
