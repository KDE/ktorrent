/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include <kgenericfactory.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

#include <qstring.h>
#include <qfile.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>

#include "scanfolder.h"
#include "scanfolderplugin.h"
#include "scanfolderprefpage.h"
#include "scanfolderpluginsettings.h"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktscanfolderplugin,KGenericFactory<kt::ScanFolderPlugin>("scanfolderplugin"))

namespace kt
{	

	ScanFolderPlugin::ScanFolderPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
	{
		Q_UNUSED(args);
		m_sf_map.setAutoDelete(true);
	}


	ScanFolderPlugin::~ScanFolderPlugin()
	{
	}

	void ScanFolderPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Scan Folder"),SYS_SNF);
		pref = new ScanFolderPrefPage(this,0);
		getGUI()->addPrefPage(pref);
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(updateScanFolders()));
		updateScanFolders();
	}

	void ScanFolderPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Scan Folder"));
		getGUI()->removePrefPage(pref);
		pref = 0;
		
		m_sf_map.clear();
	}
	
	void ScanFolderPlugin::updateScanFolders()
	{
		QStringList folders = ScanFolderPluginSettings::folders();
		
		// make sure folders end with /
		for (QStringList::iterator i = folders.begin();i !=folders.end(); i++)
		{
			if (!(*i).endsWith(bt::DirSeparator()))
				(*i) += bt::DirSeparator();
		}
		
		LoadedTorrentAction action;
		if (ScanFolderPluginSettings::actionDelete())
			action = deleteAction;
		else if (ScanFolderPluginSettings::actionMove())
			action = moveAction;
		else
			action = defaultAction;
		
		// first erase folders we don't need anymore
		bt::PtrMap<QString,ScanFolder>::iterator i = m_sf_map.begin();
		while (i != m_sf_map.end())
		{
			if (!folders.contains(i->first))
			{
				QString f = i->first;
				i++;
				m_sf_map.erase(f);
			}
			else
			{
				ScanFolder* sf = i->second;
				sf->setLoadedAction(action);
				i++;
			}
		}
		
		foreach (const QString &folder,folders)
		{
			if (m_sf_map.find(folder))
				continue;
			
			if (QDir(folder).exists())
			{
				// only add folder when it exists
				ScanFolder* sf = new ScanFolder(getCore(),folder,action);
				m_sf_map.insert(folder,sf);
			}	
		}
	}
	
	bool ScanFolderPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
