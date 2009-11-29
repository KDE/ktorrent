/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
#include <interfaces/functions.h>
#include "generalpref.h"
#include "settings.h"

namespace kt
{

	GeneralPref::GeneralPref(QWidget* parent) : PrefPageInterface(Settings::self(),i18n("Application"),"ktorrent",parent)
	{
		setupUi(this);
		kcfg_tempDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
		kcfg_saveDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
		kcfg_torrentCopyDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
		kcfg_completedDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
	}

	GeneralPref::~GeneralPref()
	{
	}

	void GeneralPref::loadSettings()
	{
		if (Settings::tempDir().toLocalFile().length() == 0)
			kcfg_tempDir->setUrl(kt::DataDir());
		else
			kcfg_tempDir->setUrl(Settings::tempDir());

		kcfg_saveDir->setEnabled(Settings::useSaveDir());
		if (Settings::saveDir().toLocalFile().length() == 0)
			kcfg_saveDir->setUrl(QDir::homePath());
		else
			kcfg_saveDir->setUrl(Settings::saveDir());

		kcfg_torrentCopyDir->setEnabled(Settings::useTorrentCopyDir());
		if (Settings::torrentCopyDir().toLocalFile().length() == 0)
			kcfg_torrentCopyDir->setUrl(QDir::homePath());
		else
			kcfg_torrentCopyDir->setUrl(Settings::torrentCopyDir());

		kcfg_completedDir->setEnabled(Settings::useCompletedDir());
		if (Settings::completedDir().toLocalFile().length() == 0)
			kcfg_completedDir->setUrl(QDir::homePath());
		else
			kcfg_completedDir->setUrl(Settings::completedDir());

//			kcfg_downloadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());
//			kcfg_uploadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());	
	}

	void GeneralPref::loadDefaults()
	{
		Settings::setTempDir(kt::DataDir());
		Settings::setSaveDir(QDir::homePath());
		Settings::setCompletedDir(QDir::homePath());
		Settings::setTorrentCopyDir(QDir::homePath());
		loadSettings();
	}

}
