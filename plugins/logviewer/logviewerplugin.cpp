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

#define NAME "Log Viewer"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktlogviewerplugin,KGenericFactory<kt::LogViewerPlugin>("ktlogviewerplugin"))

namespace kt
{

	LogViewerPlugin::LogViewerPlugin(QObject* parent,const QStringList & ) 
	: Plugin(parent,NAME, i18n("Log Viewer"),AUTHOR, EMAIL, i18n("Displays ktorrent logging output"),"utilities-log-viewer")
	{
		lv = 0;
	}


	LogViewerPlugin::~LogViewerPlugin()
	{}


	void LogViewerPlugin::load()
	{
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		lv = new LogViewer();
		pref = new LogPrefPage(0);
		getGUI()->addToolWidget(lv,"utilities-log-viewer",i18n("Log Viewer"),GUIInterface::DOCK_BOTTOM);
		getGUI()->addPrefPage(pref);
		AddLogMonitor(lv);
		applySettings();
	}

	void LogViewerPlugin::unload()
	{
		disconnect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		getGUI()->removeToolWidget(lv);
		getGUI()->removePrefPage(pref);
		RemoveLogMonitor(lv);
		lv->deleteLater();
		lv = 0;
		pref->deleteLater();
		pref = 0;
		LogFlags::finalize();
	}
	
	void LogViewerPlugin::applySettings()
	{
		LogFlags::instance().updateFlags();
	}

	bool LogViewerPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}

}
#include "logviewerplugin.moc"
