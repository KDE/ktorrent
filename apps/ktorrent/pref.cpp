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
#include <kactivelabel.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <kurl.h> 
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klineedit.h> 
#include <qlistview.h> 
#include <torrent/globals.h>
#include <util/functions.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qdir.h>

#include "pref.h"
#include "downloadpref.h"
#include "settings.h"
#include "ktorrent.h"
#include "interfaces/prefpageinterface.h"

using namespace bt;



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
}

void KTorrentPreferences::slotOk()
{
	slotApply();
	accept();
}

void KTorrentPreferences::slotApply()
{
	page_one->apply();
	page_two->apply();
	QPtrList<kt::PrefPageInterface>::iterator i = pages.begin();
	while (i != pages.end())
	{
		kt::PrefPageInterface* p = *i;
		p->apply();
		i++;
	}
	Settings::writeConfig();
	ktor.applySettings();
}

void KTorrentPreferences::updateData()
{
	page_one->updateData();
	page_two->updateData();
	QPtrList<kt::PrefPageInterface>::iterator i = pages.begin();
	while (i != pages.end())
	{
		kt::PrefPageInterface* p = *i;
		p->updateData();
		i++;
	}
}

void KTorrentPreferences::addPrefPage(kt::PrefPageInterface* prefInterface)
{
	QFrame* frame = addPage(prefInterface->getItemName(), prefInterface->getHeader(), prefInterface->getPixmap());
	QVBoxLayout* vbox = new QVBoxLayout(frame);
	vbox->setAutoAdd(true);
	prefInterface->createWidget(frame);

	pages.append(prefInterface);
}

///////////////////////////////////////////////////////

PrefPageOne::PrefPageOne(QWidget *parent) : DownloadPref(parent)
{
	updateData();
}

bool PrefPageOne::checkPorts()
{
	if (udp_tracker_port->value() == port->value())
		return false;
	else
		return true;
}

void PrefPageOne::apply()
{
	Settings::setMaxDownloads(max_downloads->value());
	Settings::setMaxConnections(max_conns->value());
	Settings::setMaxUploadRate(max_upload_rate->value());
	Settings::setMaxDownloadRate(max_download_rate->value());
	Settings::setKeepSeeding(keep_seeding->isChecked());	
	Settings::setPort(port->value());
	Settings::setUdpTrackerPort(udp_tracker_port->value());
}

void PrefPageOne::updateData()
{
	//setMinimumSize(400,400);
	max_downloads->setValue(Settings::maxDownloads());
	max_conns->setValue(Settings::maxConnections());
	max_upload_rate->setValue(Settings::maxUploadRate());
	max_download_rate->setValue(Settings::maxDownloadRate());
	keep_seeding->setChecked(Settings::keepSeeding());
	udp_tracker_port->setValue(Settings::udpTrackerPort());
	port->setValue(Settings::port());
}

//////////////////////////////////////
PrefPageTwo::PrefPageTwo(QWidget *parent) : GeneralPref(parent)
{
	updateData();
}

void PrefPageTwo::apply()
{
	Settings::setShowSystemTrayIcon(show_systray_icon->isChecked());
	Settings::setShowPeerView(m_showPeerView->isChecked());
	Settings::setShowChunkView(m_showChunkView->isChecked());
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

void PrefPageTwo::updateData()
{
	show_systray_icon->setChecked(Settings::showSystemTrayIcon());
	m_showPeerView->setChecked(Settings::showPeerView());
	m_showChunkView->setChecked(Settings::showChunkView());
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
//////////////////////////////////////////////////////////////////////////////////////// 


#include "pref.moc"
