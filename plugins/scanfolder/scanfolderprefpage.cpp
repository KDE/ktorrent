/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "scanfolderplugin.h"
#include "scanfolderprefpage.h"

#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <util/functions.h>
#include "scanfolderpluginsettings.h"



namespace kt
{

	ScanFolderPrefPage::ScanFolderPrefPage(ScanFolderPlugin* plugin,QWidget* parent)
		: PrefPageInterface(ScanFolderPluginSettings::self(),i18n("Scan Folder"), "view_sidetree",parent), m_plugin(plugin)
	{
		setupUi(this);
		connect(kcfg_actionDelete,SIGNAL(toggled(bool)),kcfg_actionMove,SLOT(setDisabled(bool)));
		connect(m_add,SIGNAL(clicked()),this,SLOT(addPressed()));
		connect(m_remove,SIGNAL(clicked()),this,SLOT(removePressed()));
	}


	ScanFolderPrefPage::~ScanFolderPrefPage()
	{}

	void ScanFolderPrefPage::loadSettings()
	{
		kcfg_actionMove->setEnabled(!ScanFolderPluginSettings::actionDelete());
		
		m_folders->clear();
		folders = ScanFolderPluginSettings::folders();
		foreach (QString f,folders)
		{
			m_folders->addItem(new QListWidgetItem(KIcon("folder"),f));
		}
	}
	
	void ScanFolderPrefPage::loadDefaults()
	{
		kcfg_actionMove->setEnabled(!ScanFolderPluginSettings::actionDelete());
		
		m_folders->clear();
		folders.clear();
	}
	
	void ScanFolderPrefPage::updateSettings()
	{
		ScanFolderPluginSettings::setFolders(folders);
		ScanFolderPluginSettings::self()->writeConfig();
		m_plugin->updateScanFolders();
	}
	
	void ScanFolderPrefPage::addPressed()
	{
		KUrl dir = KFileDialog::getExistingDirectoryUrl(KUrl(),this);
		if (dir.isValid())
		{
			QString p = dir.path();
			if (!p.endsWith(bt::DirSeparator()))
				p += bt::DirSeparator();
			m_folders->addItem(new QListWidgetItem(KIcon("folder"),p));
			folders.append(p);
		}
	}
	
	void ScanFolderPrefPage::removePressed()
	{
		QList<QListWidgetItem*> sel = m_folders->selectedItems();
		foreach (QListWidgetItem* i,sel)
		{
			folders.removeAll(i->text());
			delete i;
		}
	}

}
