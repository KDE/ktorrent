/***************************************************************************
 *   Copyright (C) 2006 by Alan Jones					   *
 *   skyphyr@gmail.com                                                     *
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
#include <kiconloader.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>
#include <util/constants.h>
#include <util/log.h>

#include <qstring.h>
#include <qfile.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>

#include "rssfeedmanager.h"
#include "rssfeedplugin.h"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktrssfeedplugin,KGenericFactory<kt::RssFeedPlugin>("rssfeedplugin"))

namespace kt
{	
	const QString NAME = "RSS Feeds";
	const QString AUTHOR = "Alan Jones";
	const QString EMAIL = "skyphyr@gmail.com";
	const QString DESCRIPTION = i18n("Automatically scans RSS feeds for torrent matching regular expressions and loads them.");

	RssFeedPlugin::RssFeedPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("RSS Feeds"),AUTHOR,EMAIL,DESCRIPTION,"player_playlist")
	{
		m_rssFeedManager = 0;
	}


	RssFeedPlugin::~RssFeedPlugin()
	{
	}

	void RssFeedPlugin::load()
	{
		//add the new tab to the gui
		KIconLoader* iload = KGlobal::iconLoader();
		m_rssFeedManager = new RssFeedManager(getCore());
		getGUI()->addTabPage(
			m_rssFeedManager,iload->loadIconSet("player_playlist", KIcon::Small),
			i18n("RSS Feeds"));
		
	}

	void RssFeedPlugin::unload()
	{
		// be sure to remove the page's tab before deleting the widget
		getGUI()->removeTabPage(m_rssFeedManager);
		delete m_rssFeedManager;
		m_rssFeedManager = 0;
	}
	
	bool RssFeedPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
}

