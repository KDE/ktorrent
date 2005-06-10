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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <libtorrent/globals.h>

#include "pref.h"
#include "downloadpref.h"
#include "settings.h"
#include "ktorrent.h"


class KTorrentPrefPageOne : public DownloadPref
{
public:
	KTorrentPrefPageOne(QWidget *parent = 0);
	
	void apply();
};

KTorrentPreferences::KTorrentPreferences(KTorrent & ktor)
	: KDialogBase(IconList, i18n("Preferences"),Ok|Apply|Cancel, Ok),ktor(ktor)
{
	// this is the base class for your preferences dialog.  it is now
	// a Treelist dialog.. but there are a number of other
	// possibilities (including Tab, Swallow, and just Plain)
	enableButtonSeparator(true);
	setInitialSize(QSize(800,300),true);
	QFrame *frame = addPage(i18n("Downloads"), i18n("Download Options"));
	m_page_one = new KTorrentPrefPageOne(frame);
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
	m_page_one->apply();
	ktor.applySettings();
}



KTorrentPrefPageOne::KTorrentPrefPageOne(QWidget *parent) : DownloadPref(parent)
{
	setMinimumSize(400,400);
	
	max_downloads->setValue(Settings::maxDownloads());
	max_conns->setValue(Settings::maxConnections());
	max_upload_rate->setValue(Settings::maxUploadRate());
	keep_seeding->setChecked(Settings::keepSeeding());
	show_systray_icon->setChecked(Settings::showSystemTrayIcon());
	port->setValue(Settings::port());
	KURLRequester* u = temp_dir;
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
}

void KTorrentPrefPageOne::apply()
{
	Settings::setMaxDownloads(max_downloads->value());
	Settings::setMaxConnections(max_conns->value());
	Settings::setMaxUploadRate(max_upload_rate->value());
	Settings::setKeepSeeding(keep_seeding->isChecked());
	Settings::setShowSystemTrayIcon(show_systray_icon->isChecked());
	Settings::setPort(port->value());
	QString ourl = Settings::tempDir();
	
	KURLRequester* u = temp_dir;
	if (ourl != u->url())
	{
		Settings::setTempDir(u->url());
	}
	
	Settings::writeConfig();
}


#include "pref.moc"
