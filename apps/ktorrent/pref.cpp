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
#include <kcombobox.h>
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
#include <kresolver.h>

#include "downloadpref.h"
#include "generalpref.h"
#include "pref.h"
#include "downloadpref.h"
#include "settings.h"
#include "ktorrent.h"


using namespace bt;
using namespace KNetwork;


KTorrentPreferences::KTorrentPreferences(KTorrent & ktor)
	: KDialogBase(IconList, i18n("Preferences"),Ok|Apply|Cancel, Ok),ktor(ktor)
{
	validation_err = false;
	enableButtonSeparator(true);
		
	page_one = new PrefPageOne();
	page_two = new PrefPageTwo();
	addPrefPage(page_one);
	addPrefPage(page_two);
}

KTorrentPreferences::~KTorrentPreferences()
{
	delete page_one;
	delete page_two;
}

void KTorrentPreferences::slotOk()
{
	slotApply();
	if (!validation_err)
		accept();
}

void KTorrentPreferences::slotApply()
{
	validation_err = false;
	QMap<kt::PrefPageInterface*,QFrame*>::iterator i = pages.begin();
	while (i != pages.end())
	{
		kt::PrefPageInterface* p = i.key();
		if (!p->apply())
		{
			validation_err = true;
			return;
		}
		i++;
	}
	Settings::writeConfig();
	ktor.applySettings();
}

void KTorrentPreferences::updateData()
{
	QMap<kt::PrefPageInterface*,QFrame*>::iterator i = pages.begin();
	while (i != pages.end())
	{
		kt::PrefPageInterface* p = i.key();
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

	pages.insert(prefInterface,frame);
}

void KTorrentPreferences::removePrefPage(kt::PrefPageInterface* pp)
{
	if (!pages.contains(pp))
		return;
	
	QFrame* fr = pages[pp];
	pages.remove(pp);
	pp->deleteWidget();
	delete fr;
}

///////////////////////////////////////////////////////

PrefPageOne::PrefPageOne() : kt::PrefPageInterface(i18n("Downloads"), i18n("Download Options"),KGlobal::iconLoader()->loadIcon("down",KIcon::NoGroup)),dp(0)
{
}

PrefPageOne::~ PrefPageOne()
{
	delete dp;
}

void PrefPageOne::createWidget(QWidget* parent)
{
	dp = new DownloadPref(parent);
	updateData();
}

bool PrefPageOne::apply()
{
	Settings::setMaxDownloads(dp->max_downloads->value());
	Settings::setMaxSeeds(dp->max_seeds->value());
	Settings::setMaxConnections(dp->max_conns->value());
	Settings::setMaxUploadRate(dp->max_upload_rate->value());
	Settings::setMaxDownloadRate(dp->max_download_rate->value());
	Settings::setKeepSeeding(dp->keep_seeding->isChecked());
	Settings::setPort(dp->port->value());
	Settings::setNumUploadSlots(dp->num_upload_slots->value());
	if (Settings::dhtSupport() && dp->udp_tracker_port->value() == Settings::dhtPort())
	{
		QString msg = i18n("The DHT port needs to be different then the UDP tracker port !");
		KMessageBox::error(0,msg,i18n("Error"));
		return false;
	}
	Settings::setUdpTrackerPort(dp->udp_tracker_port->value());
	return true;
}

void PrefPageOne::updateData()
{
	//setMinimumSize(400,400);
	dp->max_downloads->setValue(Settings::maxDownloads());
	dp->max_seeds->setValue(Settings::maxSeeds());
	dp->max_conns->setValue(Settings::maxConnections());
	dp->max_upload_rate->setValue(Settings::maxUploadRate());
	dp->max_download_rate->setValue(Settings::maxDownloadRate());
	dp->keep_seeding->setChecked(Settings::keepSeeding());
	dp->udp_tracker_port->setValue(Settings::udpTrackerPort());
	dp->port->setValue(Settings::port());
	dp->num_upload_slots->setValue(Settings::numUploadSlots());
}

void PrefPageOne::deleteWidget()
{
	delete dp;
}

//////////////////////////////////////
PrefPageTwo::PrefPageTwo() :
		kt::PrefPageInterface(i18n("General"), i18n("General Options"),
							  KGlobal::iconLoader()->loadIcon("package_settings",KIcon::NoGroup)),gp(0)
{
}

PrefPageTwo::~PrefPageTwo()
{
	delete gp;
}

void PrefPageTwo::createWidget(QWidget* parent)
{
	gp = new GeneralPref(parent);
	updateData();
	connect(gp->autosave_downloads_check,SIGNAL(toggled(bool)),
			this,SLOT(autosaveChecked(bool )));
	connect(gp->custom_ip_check,SIGNAL(toggled(bool)),
			this,SLOT(customIPChecked(bool )));
	connect(gp->use_dht,SIGNAL(toggled(bool)),
			this,SLOT(dhtChecked( bool )));
}

bool PrefPageTwo::apply()
{
	Settings::setShowSystemTrayIcon(gp->show_systray_icon->isChecked());
	QString ourl = Settings::tempDir();
	
	KURLRequester* u = gp->temp_dir;
	if (ourl != u->url())
	{
		Settings::setTempDir(u->url());
	}

	Settings::setSaveDir(gp->autosave_location->url());
	bool useSaveDir = gp->autosave_downloads_check->isChecked();
	Settings::setUseSaveDir(useSaveDir);

        bool useExternalIP = gp->custom_ip_check->isChecked();
	
	Settings::setUseExternalIP(useExternalIP);
	QString externalIP = gp->custom_ip->text();
	Settings::setExternalIP(externalIP);
			
	if (useExternalIP)
	{

		KResolverResults res = KResolver::resolve(externalIP, QString::null);
		if (res.error())
		{
			QString err = KResolver::errorString(res.error());
			QString msg = i18n("Cannot lookup %1: %2\n"
					"Please provide a valid IP address or hostname.").arg(externalIP).arg(err);
			KMessageBox::error(0,msg,i18n("Error"));
			return false;
		}
	}
	
	Settings::setMemoryUsage(gp->mem_usage->currentItem());
	Settings::setGuiUpdateInterval(gp->gui_interval->currentItem());
	
	if (gp->use_dht->isChecked() && gp->dht_port->value() == Settings::udpTrackerPort())
	{
		QString msg = i18n("The DHT port needs to be different then the UDP tracker port !");
		KMessageBox::error(0,msg,i18n("Error"));
		return false;
	}
	
	Settings::setDhtSupport(gp->use_dht->isChecked());
	Settings::setDhtPort(gp->dht_port->value());
	return true;
}

void PrefPageTwo::autosaveChecked(bool on)
{
	gp->autosave_location->setEnabled(on);
}

void PrefPageTwo::customIPChecked(bool on)
{
	gp->custom_ip->setEnabled(on);
	gp->custom_ip_label->setEnabled(on);
}

void PrefPageTwo::dhtChecked(bool on)
{
	gp->dht_port->setEnabled(on);
}

void PrefPageTwo::updateData()
{
	gp->show_systray_icon->setChecked(Settings::showSystemTrayIcon());
	KURLRequester* u = gp->temp_dir;
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

	u = gp->autosave_location;
	u->fileDialog()->setMode(KFile::Directory);
	
	bool useSaveDir = Settings::useSaveDir();
        QString saveDir = Settings::saveDir();

	gp->autosave_downloads_check->setChecked(useSaveDir);
	u->setEnabled(useSaveDir);

	u->setURL(!saveDir.isEmpty() ? saveDir : QDir::homeDirPath());

	gp->custom_ip->setText(Settings::externalIP());	

	bool useExternalIP = Settings::useExternalIP();
	gp->custom_ip_check->setChecked(useExternalIP);
	gp->custom_ip->setEnabled(useExternalIP);
	gp->custom_ip_label->setEnabled(useExternalIP);
	
	gp->mem_usage->setCurrentItem(Settings::memoryUsage());
	gp->gui_interval->setCurrentItem(Settings::guiUpdateInterval());
	
	gp->use_dht->setChecked(Settings::dhtSupport());
	gp->dht_port->setValue(Settings::dhtPort());
	gp->dht_port->setEnabled(Settings::dhtSupport());
}

void PrefPageTwo::deleteWidget()
{
	delete gp;
}

#include "pref.moc"
