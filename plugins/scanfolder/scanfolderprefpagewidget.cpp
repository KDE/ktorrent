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
#include "scanfolderprefpagewidget.h"
#include "scanfolderpluginsettings.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qfile.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

namespace kt
{
	ScanFolderPrefPageWidget::ScanFolderPrefPageWidget(QWidget *parent, const char *name)
			:SfPrefPageWidgetBase(parent, name)
	{
		use1->setChecked(ScanFolderPluginSettings::useFolder1());
		use2->setChecked(ScanFolderPluginSettings::useFolder2());
		use3->setChecked(ScanFolderPluginSettings::useFolder3());

		url1->setURL(ScanFolderPluginSettings::folder1());
		url2->setURL(ScanFolderPluginSettings::folder2());
		url3->setURL(ScanFolderPluginSettings::folder3());

		openSilently->setChecked(ScanFolderPluginSettings::openSilently());
		deleteCheck->setChecked(ScanFolderPluginSettings::actionDelete());
		moveCheck->setChecked(ScanFolderPluginSettings::actionMove());

		url1->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
		url2->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
		url3->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
	}

	void ScanFolderPrefPageWidget::apply()
	{
		bool usesf1 = use1->isChecked();
		bool usesf2 = use2->isChecked();
		bool usesf3 = use3->isChecked();

		QString sfPath1 = url1->url();
		QString sfPath2 = url2->url();
		QString sfPath3 = url3->url();

		bool silently = openSilently->isChecked();
		bool deleteChecked = deleteCheck->isChecked();
		bool moveChecked = moveCheck->isChecked();

		ScanFolderPluginSettings::setOpenSilently(silently);
		ScanFolderPluginSettings::setActionDelete(deleteChecked);
		ScanFolderPluginSettings::setActionMove(moveChecked);
		
		QString message = i18n( "ScanFolder - Folder %1: Invalid URL or folder does not exist. Please, choose a valid directory." );
		if(!QFile::exists(sfPath1) && usesf1)
		{
			KMessageBox::sorry(0, message.arg( 1 ) );
			usesf1 = false;
		}
		else
			ScanFolderPluginSettings::setFolder1(sfPath1);

		if(!QFile::exists(sfPath2) && usesf2)
		{
			KMessageBox::sorry(0, message.arg( 2 ) );
			usesf2 = false;
		}
		else
			ScanFolderPluginSettings::setFolder2(sfPath2);

		if(!QFile::exists(sfPath3) && usesf3)
		{
			KMessageBox::sorry(0, message.arg( 3 ) );
			usesf3 = false;
		}
		else
			ScanFolderPluginSettings::setFolder3(sfPath3);


		ScanFolderPluginSettings::setUseFolder1(usesf1);
		ScanFolderPluginSettings::setUseFolder2(usesf2);
		ScanFolderPluginSettings::setUseFolder3(usesf3);

		ScanFolderPluginSettings::writeConfig();
	}

}
#include "scanfolderprefpagewidget.moc"
