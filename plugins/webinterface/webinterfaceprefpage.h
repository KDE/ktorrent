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

#ifndef WEBINTERFACEPREFPAGE_H
#define WEBINTERFACEPREFPAGE_H
#include <interfaces/prefpageinterface.h>
#include "webinterfaceprefwidget.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
namespace kt
{

	/**
	 * WebInterface plugin preferences page
	 * @author Diego R. Brogna <dierbro@gmail.com>
	*/
	class WebInterfacePlugin;
	class WebInterfacePrefPage : public PrefPageInterface
	{
		public:
			WebInterfacePrefPage(WebInterfacePlugin* plugin);
			virtual ~WebInterfacePrefPage();

			virtual bool apply();
			virtual void createWidget(QWidget* parent);
			virtual void updateData();
			virtual void deleteWidget();
			
		private:
			WebInterfacePrefWidget* m_widget;
			WebInterfacePlugin* w_plugin;
	};

}

#endif
