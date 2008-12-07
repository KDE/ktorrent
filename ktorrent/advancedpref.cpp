/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include "advancedpref.h"
#include "settings.h"

namespace kt
{
	AdvancedPref::AdvancedPref(QWidget* parent) : PrefPageInterface(Settings::self(),i18n("Advanced"),"configure",parent)
	{
		setupUi(this);
		connect(kcfg_doUploadDataCheck,SIGNAL(toggled(bool)),this,SLOT(onUploadDataCheckToggled(bool)));
		connect(kcfg_diskPrealloc,SIGNAL(toggled(bool)),this,SLOT(onDiskPreallocToggled(bool)));
	}

	AdvancedPref::~AdvancedPref()
	{
	}

	void AdvancedPref::loadSettings()
	{
		kcfg_maxCorruptedBeforeRecheck->setEnabled(Settings::autoRecheck());
		kcfg_useMaxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck());
		kcfg_maxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck() && Settings::useMaxSizeForUploadDataCheck());

		kcfg_fullDiskPreallocMethod->setEnabled(Settings::diskPrealloc() && Settings::fullDiskPrealloc());
		kcfg_fullDiskPrealloc->setEnabled(Settings::diskPrealloc());
	}

	void AdvancedPref::loadDefaults()
	{
		loadSettings();
	}

	void AdvancedPref::onUploadDataCheckToggled(bool on)
	{
		kcfg_useMaxSizeForUploadDataCheck->setEnabled(on);
		kcfg_maxSizeForUploadDataCheck->setEnabled(on&&kcfg_useMaxSizeForUploadDataCheck->isChecked());
	}
	
	void AdvancedPref::onDiskPreallocToggled(bool on)
	{
		kcfg_fullDiskPreallocMethod->setEnabled(on&&kcfg_fullDiskPrealloc->isChecked());
		kcfg_fullDiskPrealloc->setEnabled(on);

	}

}

#include "advancedpref.moc"

// kate: space-indent on; replace-tabs off; mixed-indent off;
