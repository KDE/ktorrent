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
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include "logviewerplugin.h"
#include "logviewer.h"

#define NAME "logviewerplugin"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktlogviewerplugin,KGenericFactory<kt::LogViewerPlugin>("ktlogviewerplugin"))

namespace kt
{

	LogViewerPlugin::LogViewerPlugin(QObject* parent, const char* qt_name, const QStringList& args)
	: Plugin(parent, qt_name, args, NAME, AUTHOR, EMAIL,i18n("Shows logging output in a widget"))
	{
		lv = 0;
	}


	LogViewerPlugin::~LogViewerPlugin()
	{}


	void LogViewerPlugin::load()
	{
		lv = new LogViewer();
		this->getGUI()->addWidgetBelowView(lv);
		bt::Log & lg = Out();
		lg.addMonitor(lv);
	}

	void LogViewerPlugin::unload()
	{
		this->getGUI()->removeWidgetBelowView(lv);
		bt::Log & lg = Out();
		lg.removeMonitor(lv);
		delete lv;
		lv = 0;
	}

}
#include "logviewerplugin.moc"
