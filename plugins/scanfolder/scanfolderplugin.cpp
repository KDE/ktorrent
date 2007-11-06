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
#include <util/log.h>

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
	const QString NAME = "Scan Folder";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("Automatically scans directories for torrent files and loads them.");

	ScanFolderPlugin::ScanFolderPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("Scan Folder"),AUTHOR,EMAIL,DESCRIPTION,"view_sidetree")
	{
// 		setXMLFile("ktscanfolderpluginui.rc");
		m_sf1 = 0;
		m_sf2 = 0;
		m_sf3 = 0;
	}


	ScanFolderPlugin::~ScanFolderPlugin()
	{
	}

	void ScanFolderPlugin::load()
	{
		pref = new ScanFolderPrefPage(this);
		getGUI()->addPrefPage(pref);
		updateScanFolders();
	}

	void ScanFolderPlugin::unload()
	{
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;
		
		if(m_sf1)
			delete m_sf1;
		m_sf1 = 0;
		
		if(m_sf2)
			delete m_sf2;
		m_sf2 = 0;
		
		if(m_sf3)
			delete m_sf3;
		m_sf3 = 0;
	}
	
	void ScanFolderPlugin::updateScanFolders()
	{
		QString sfPath1 = ScanFolderPluginSettings::folder1();
		QString sfPath2 = ScanFolderPluginSettings::folder2();
		QString sfPath3 = ScanFolderPluginSettings::folder3();
		
		bool valid1 = QFile::exists(sfPath1);
		bool valid2 = QFile::exists(sfPath2);
		bool valid3 = QFile::exists(sfPath3);
		
		bool usesf1 = ScanFolderPluginSettings::useFolder1() && valid1;
		bool usesf2 = ScanFolderPluginSettings::useFolder2() && valid2;
		bool usesf3 = ScanFolderPluginSettings::useFolder3() && valid3;
		
		bool silently = ScanFolderPluginSettings::openSilently();
		
		LoadedTorrentAction action;
		
		if(ScanFolderPluginSettings::actionDelete())
			action = deleteAction;
		else if(ScanFolderPluginSettings::actionMove())
			action = moveAction;
		else
			action = defaultAction;
		
		
		if(usesf1)
		{
			if(!m_sf1)
				m_sf1 = new ScanFolder(getCore(), sfPath1, action, silently);
			else
			{
				m_sf1->setFolderUrl(sfPath1);
				m_sf1->setLoadedAction(action);
				m_sf1->setOpenSilently(silently);
			}
		}
		else
		{
			if(m_sf1)
				delete m_sf1;
			m_sf1 = 0;
		}
		
		if(usesf2)
		{
			if(!m_sf2)
				m_sf2 = new ScanFolder(getCore(), sfPath1, action, silently);
			else
			{
				m_sf2->setFolderUrl(sfPath1);
				m_sf2->setLoadedAction(action);
				m_sf2->setOpenSilently(silently);
			}
		}
		else
		{
			if(m_sf2)
				delete m_sf2;
			m_sf2 = 0;
		}
		
		if(usesf3)
		{
			if(!m_sf3)
				m_sf3 = new ScanFolder(getCore(), sfPath1, action, silently);
			else
			{
				m_sf3->setFolderUrl(sfPath1);
				m_sf3->setLoadedAction(action);
				m_sf3->setOpenSilently(silently);
			}
		}
		else
		{
			if(m_sf3)
				delete m_sf3;
			m_sf3 = 0;
		}
		
		//update config file
		if(!valid1)
			ScanFolderPluginSettings::setUseFolder1(false);
		if(!valid2)
			ScanFolderPluginSettings::setUseFolder2(false);
		if(!valid3)
			ScanFolderPluginSettings::setUseFolder3(false);
		
		ScanFolderPluginSettings::writeConfig();
			
	}
	
	bool ScanFolderPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
