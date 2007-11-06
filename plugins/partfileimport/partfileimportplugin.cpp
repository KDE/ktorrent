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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstdaction.h>
#include <kpopupmenu.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "partfileimportplugin.h"
#include "importdialog.h"

#define NAME "Import"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"



K_EXPORT_COMPONENT_FACTORY(ktpartfileimportplugin,KGenericFactory<kt::PartFileImportPlugin>("ktpartfileimportplugin"))

namespace kt
{

	PartFileImportPlugin::PartFileImportPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("Import"),AUTHOR,EMAIL,i18n("Imports partially or fully downloaded torrents from other clients"),"ktplugins")
	{
		setXMLFile("ktpartfileimportpluginui.rc");
		import_action = 0;
	}


	PartFileImportPlugin::~PartFileImportPlugin()
	{}


	void PartFileImportPlugin::load()
	{
		import_action = new KAction(i18n("Import existing download" ), 0, this,
									 SLOT(onImport()), actionCollection(), "partfileimport" );
	}

	void PartFileImportPlugin::unload()
	{
		delete import_action;
		import_action = 0;
	}
	
	void PartFileImportPlugin::onImport()
	{
		ImportDialog dlg(getCore(),0,0,true);
		dlg.exec();
	}
	
	bool PartFileImportPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}

}
#include "partfileimportplugin.moc"
