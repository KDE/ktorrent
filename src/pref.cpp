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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <libtorrent/globals.h>
#include <libutil/functions.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qdir.h>

#include "pref.h"
#include "downloadpref.h"
#include "settings.h"
#include "ktorrent.h"




KTorrentPreferences::KTorrentPreferences(KTorrent & ktor)
	: KDialogBase(IconList, i18n("Preferences"),Ok|Apply|Cancel, Ok),ktor(ktor)
{
	enableButtonSeparator(true);
		
	KIconLoader* iload = KGlobal::iconLoader();
	
	QFrame* frame = addPage(i18n("Downloads"), i18n("Download Options"),
							iload->loadIcon("down",KIcon::NoGroup));
		
	QVBoxLayout* vbox = new QVBoxLayout(frame);
	vbox->setAutoAdd(true);
	page_one = new PrefPageOne(frame);

	frame = addPage(i18n("General"), i18n("General Options"),
					iload->loadIcon("package_settings",KIcon::NoGroup));
	vbox = new QVBoxLayout(frame);
	vbox->setAutoAdd(true);
	page_two = new PrefPageTwo(frame);
	
	connect(this,SIGNAL(applyClicked()),this,SLOT(applyPressed()));
	connect(this,SIGNAL(okClicked()),this,SLOT(okPressed()));
}

void KTorrentPreferences::okPressed()
{
	applyPressed();
	accept();
}

void KTorrentPreferences::applyPressed()
{
	page_one->apply();
	page_two->apply();
	Settings::writeConfig();
	ktor.applySettings();
}

///////////////////////////////////////////////////////

PrefPageOne::PrefPageOne(QWidget *parent) : DownloadPref(parent)
{
	//setMinimumSize(400,400);
	max_downloads->setValue(Settings::maxDownloads());
	max_conns->setValue(Settings::maxConnections());
	max_upload_rate->setValue(Settings::maxUploadRate());
	max_download_rate->setValue(Settings::maxDownloadRate());
	keep_seeding->setChecked(Settings::keepSeeding());
	port->setValue(Settings::port());
}



void PrefPageOne::apply()
{
	Settings::setMaxDownloads(max_downloads->value());
	Settings::setMaxConnections(max_conns->value());
	Settings::setMaxUploadRate(max_upload_rate->value());
	Settings::setMaxDownloadRate(max_download_rate->value());
	Settings::setKeepSeeding(keep_seeding->isChecked());	
	Settings::setPort(port->value());
}

//////////////////////////////////////
PrefPageTwo::PrefPageTwo(QWidget *parent) : GeneralPref(parent)
{
	show_systray_icon->setChecked(Settings::showSystemTrayIcon());
	KURLRequester* u = temp_dir;
	u->fileDialog()->setMode(KFile::Directory);
	if (Settings::tempDir() == QString::null)
	{
		QString data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
		if (!data_dir.endsWith(bt::DirSeparator()))
			data_dir += bt::DirSeparator();
		u->setURL(data_dir);
	}
	else
	{
		u->setURL(Settings::tempDir());
	}

	u = autosave_location;
	u->fileDialog()->setMode(KFile::Directory);
	if (Settings::saveDir() == QString::null)
	{
		autosave_downloads_check->setChecked(false);
		u->setEnabled(false);
		u->clear();
	}
	else
	{
		autosave_downloads_check->setChecked(true);
		u->setURL(QDir::homeDirPath());
		u->setURL(Settings::saveDir());
		u->setEnabled(true);
	}
	connect(autosave_downloads_check,SIGNAL(toggled(bool)),
			this,SLOT(autosaveChecked(bool )));
}

void PrefPageTwo::apply()
{
	Settings::setShowSystemTrayIcon(show_systray_icon->isChecked());
	QString ourl = Settings::tempDir();
	
	KURLRequester* u = temp_dir;
	if (ourl != u->url())
	{
		Settings::setTempDir(u->url());
	}

	if (autosave_downloads_check->isChecked())
	{
		u = autosave_location;
		Settings::setSaveDir(u->url());
	}
	else
	{
		Settings::setSaveDir(QString::null);
	}
}

void PrefPageTwo::autosaveChecked(bool on)
{
	KURLRequester* u = autosave_location;
	if (on)
	{
		u->setEnabled(true);
		if (Settings::saveDir() == QString::null)
			u->setURL(QDir::homeDirPath());
		else
			u->setURL(Settings::saveDir());
	}
	else
	{
		u->setEnabled(false);
		u->clear();
	}
}

#include "pref.moc"
