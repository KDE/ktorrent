/***************************************************************************
 *   Copyright (C) 2005 by Adam Treat                                      *
 *   treat@kde.org                                                         *
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
#include <kurl.h>
#include <kglobal.h>
#include <kstartupinfo.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include "app.h"
#include "gui.h"

namespace kt
{
	GUI* App::main_widget = 0;
	
	App::App() : KUniqueApplication()
	{
	}

	App::~App()
	{
	}
	

	int App::newInstance()
	{
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();		
		kt::GUI *widget = 0;
		if (!main_widget)
		{
			bt::InitLog(kt::DataDir() + "log",true);
			widget = new kt::GUI();
			setTopWidget(widget);
			main_widget = widget;
		}
		else
		{
			widget = main_widget;
			widget->show();
		}
		
		if (widget)
		{
			for (int i = 0; i < args->count(); i++)
			{
				if (args->isSet("silent"))
					widget->loadSilently(args->url(i));
				else
					widget->load(args->url(i));
			}
		}
		args->clear();
		return 0;
	}
}

#include "app.moc"

