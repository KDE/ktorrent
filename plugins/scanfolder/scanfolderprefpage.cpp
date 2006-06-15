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
#include <kstandarddirs.h>
#include <kiconloader.h>



namespace kt
{

	ScanFolderPrefPage::ScanFolderPrefPage(ScanFolderPlugin* plugin)
		: PrefPageInterface(i18n("ScanFolder"), i18n("ScanFolder Options"),
							KGlobal::iconLoader()->loadIcon("view_sidetree",KIcon::NoGroup)), m_plugin(plugin)
	{}


	ScanFolderPrefPage::~ScanFolderPrefPage()
	{}

	bool ScanFolderPrefPage::apply()
	{
		if(m_widget)
			m_widget->apply();
		
		m_plugin->updateScanFolders();
		
		return true;
	}

	void ScanFolderPrefPage::createWidget(QWidget* parent)
	{
		m_widget = new ScanFolderPrefPageWidget(parent);
	}

	void ScanFolderPrefPage::updateData()
	{
	}

	void ScanFolderPrefPage::deleteWidget()
	{
		delete m_widget;
	}

}
