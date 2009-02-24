/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "logviewerplugin.h"
#include "logviewer.h"
#include "logprefpage.h"
#include "logflags.h"
#include "logviewerpluginsettings.h"


using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktlogviewerplugin,KGenericFactory<kt::LogViewerPlugin>("ktlogviewerplugin"))

namespace kt
{
	
	//////////////////////////////////////////////////

	LogViewerPlugin::LogViewerPlugin(QObject* parent,const QStringList & ) : Plugin(parent)
	{
		lv = 0;
		flags = 0;
	}


	LogViewerPlugin::~LogViewerPlugin()
	{}


	void LogViewerPlugin::load()
	{
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		flags = new LogFlags();
		lv = new LogViewer(flags);
		pref = new LogPrefPage(flags,0);
		getGUI()->addActivity(lv);
		getGUI()->addPrefPage(pref);
		AddLogMonitor(lv);
		applySettings();
	}

	void LogViewerPlugin::unload()
	{
		pref->saveState();
		disconnect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		getGUI()->removePrefPage(pref);
		getGUI()->removeActivity(lv);
		RemoveLogMonitor(lv);
		lv = 0;
		pref = 0;
		delete flags;
		flags = 0;
	}
	
	void LogViewerPlugin::applySettings()
	{
		lv->setRichText(LogViewerPluginSettings::useRichText());
	}

	bool LogViewerPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}

}
#include "logviewerplugin.moc"
