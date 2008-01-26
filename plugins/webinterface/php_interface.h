  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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

#ifndef PHP_INTERFACE_H
#define PHP_INTERFACE_H
		
#include <qstring.h>
#include <torrent/peermanager.h>
#include <settings.h>
#include <torrent/queuemanager.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>

	/**
	 * @author Diego R. Brogna
	 */
namespace kt
{
	class PhpCodeGenerator
	{
		public:
			PhpCodeGenerator(CoreInterface *c);
			virtual ~PhpCodeGenerator(){}
			
			void downloadStatus(QTextStream & out);
			void globalInfo(QTextStream & out);
		private:
			CoreInterface *core;
	};
	
	class PhpActionExec
	{
		public:
			PhpActionExec(CoreInterface *c);
			virtual ~PhpActionExec(){};
			
			bool exec(KURL & url,bool & shutdown);
		private:	
			CoreInterface *core;
	};
	
	class PhpInterface: public PhpCodeGenerator, public PhpActionExec
	{
		public:
			PhpInterface(CoreInterface *c);
			//~PhpInterface{};
	};
}

#endif
