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
#include <qslider.h>
#include <kresolver.h>

#include "downloadpref.h"
#include "generalpref.h"
#include "pref.h"
#include "downloadpref.h"
#include "advancedpref.h"
#include "settings.h"
#include "ktorrent.h"


using namespace bt;

using namespace KNetwork;


KTorrentPreferences::KTorrentPreferences(KTorrent & ktor)
		: KDialogBase(IconList, i18n("Preferences"), Ok | Apply | Cancel, Ok), ktor(ktor)
{
	validation_err = false;
	enableButtonSeparator(true);

	page_one = new DownloadPrefPage();
	page_two = new GeneralPrefPage();
	page_three = new AdvancedPrefPage();
	addPrefPage(page_one);
	addPrefPage(page_two);
	addPrefPage(page_three);
}

KTorrentPreferences::~KTorrentPreferences()
{
	delete page_one;
	delete page_two;
	delete page_three;
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
	QMap<kt::PrefPageInterface*, QFrame*>::iterator i = pages.begin();

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

	ktor.applySettings(true);
}

void KTorrentPreferences::updateData()
{
	QMap<kt::PrefPageInterface*, QFrame*>::iterator i = pages.begin();

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

	pages.insert(prefInterface, frame);
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

DownloadPrefPage::DownloadPrefPage() : kt::PrefPageInterface(i18n("Downloads"), i18n("Download Options"), KGlobal::iconLoader()->loadIcon("down", KIcon::NoGroup)), dp(0)
{}

DownloadPrefPage::~ DownloadPrefPage()
{
	delete dp;
}

void DownloadPrefPage::createWidget(QWidget* parent)
{
	dp = new DownloadPref(parent);
	updateData();
}

bool DownloadPrefPage::apply()
{
	Settings::setMaxDownloads(dp->max_downloads->value());
	Settings::setMaxSeeds(dp->max_seeds->value());
	Settings::setStartDownloadsOnLowDiskSpace(dp->cmbDiskSpace->currentItem());
	Settings::setMaxConnections(dp->max_conns->value());
	Settings::setMaxTotalConnections(dp->max_total_conns->value());
	Settings::setMaxUploadRate(dp->max_upload_rate->value());
	Settings::setMaxDownloadRate(dp->max_download_rate->value());
	Settings::setMaxRatio(dp->num_max_ratio->value());
	Settings::setKeepSeeding(dp->keep_seeding->isChecked());
	Settings::setPort(dp->port->value());
	Settings::setNumUploadSlots(dp->num_upload_slots->value());
	Settings::setMinDiskSpace(dp->intMinDiskSpace->value());
	Settings::setMaxSeedTime(dp->max_seed_time->value());

	if (Settings::dhtSupport() && dp->udp_tracker_port->value() == Settings::dhtPort())
	{
		QString msg = i18n("The DHT port needs to be different from the UDP tracker port!");
		KMessageBox::error(0, msg, i18n("Error"));
		return false;
	}

	Settings::setUdpTrackerPort(dp->udp_tracker_port->value());

	return true;
}

void DownloadPrefPage::updateData()
{
	//setMinimumSize(400,400);
	dp->max_downloads->setValue(Settings::maxDownloads());
	dp->max_seeds->setValue(Settings::maxSeeds());
	dp->cmbDiskSpace->setCurrentItem(Settings::startDownloadsOnLowDiskSpace());
	dp->max_conns->setValue(Settings::maxConnections());
	dp->max_total_conns->setValue(Settings::maxTotalConnections());
	dp->max_upload_rate->setValue(Settings::maxUploadRate());
	dp->max_download_rate->setValue(Settings::maxDownloadRate());
	dp->num_max_ratio->setValue(Settings::maxRatio());
	dp->keep_seeding->setChecked(Settings::keepSeeding());
	dp->udp_tracker_port->setValue(Settings::udpTrackerPort());
	dp->port->setValue(Settings::port());
	dp->num_upload_slots->setValue(Settings::numUploadSlots());
	dp->intMinDiskSpace->setValue(Settings::minDiskSpace());
	dp->max_seed_time->setValue(Settings::maxSeedTime());
}

void DownloadPrefPage::deleteWidget()
{
	delete dp;
	dp = 0;
}

//////////////////////////////////////
GeneralPrefPage::GeneralPrefPage() :
		kt::PrefPageInterface(i18n("General"), i18n("General Options"),
							  KGlobal::iconLoader()->loadIcon("package_settings", KIcon::NoGroup)), gp(0)
{}

GeneralPrefPage::~GeneralPrefPage()
{
	delete gp;
}

void GeneralPrefPage::createWidget(QWidget* parent)
{
	gp = new GeneralPref(parent);
	updateData();
	connect(gp->custom_ip_check, SIGNAL(toggled(bool)),
			this, SLOT(customIPChecked(bool)));
	connect(gp->use_dht, SIGNAL(toggled(bool)),
			this, SLOT(dhtChecked(bool)));
	connect(gp->use_encryption, SIGNAL(toggled(bool)),
			this, SLOT(useEncryptionChecked(bool)));
}

bool GeneralPrefPage::apply()
{
	Settings::setShowSystemTrayIcon(gp->show_systray_icon->isChecked());
	Settings::setShowSpeedBarInTrayIcon(gp->show_speedbar->isChecked());
	Settings::setDownloadBandwidth(gp->downloadBandwidth->value());
	Settings::setUploadBandwidth(gp->uploadBandwidth->value());
	Settings::setShowPopups(gp->show_popups->isChecked());
	QString ourl = Settings::tempDir();

	KURLRequester* u = gp->temp_dir;

	if (ourl != u->url())
	{
		Settings::setTempDir(u->url());
	}

	Settings::setSaveDir(gp->autosave_location->url());

	bool useSaveDir = gp->autosave_downloads_check->isChecked();
	Settings::setUseSaveDir(useSaveDir);

	//check completed dir
	Settings::setCompletedDir(gp->urlCompletedDir->url());

	bool useCompletedDir = gp->checkCompletedDir->isChecked();
	Settings::setUseCompletedDir(useCompletedDir);
	
	//.torrent copy dir
	bool useTorrentCopyDir = gp->checkTorrentDir->isChecked();
	Settings::setUseTorrentCopyDir(useTorrentCopyDir);
	Settings::setTorrentCopyDir(gp->urlTorrentDir->url());

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
			KMessageBox::error(0, msg, i18n("Error"));
			return false;
		}
	}



	if (gp->use_dht->isChecked() && gp->dht_port->value() == Settings::udpTrackerPort())
	{
		QString msg = i18n("The DHT port needs to be different from the UDP tracker port!");
		KMessageBox::error(0, msg, i18n("Error"));
		return false;
	}

	Settings::setDhtSupport(gp->use_dht->isChecked());

	Settings::setDhtPort(gp->dht_port->value());
	Settings::setUseEncryption(gp->use_encryption->isChecked());
	Settings::setAllowUnencryptedConnections(gp->allow_unencrypted->isChecked());
	return true;
}

void GeneralPrefPage::useEncryptionChecked(bool on)
{
	gp->allow_unencrypted->setEnabled(on);
}

void GeneralPrefPage::autosaveChecked(bool on)
{
	gp->autosave_location->setEnabled(on);
}

void GeneralPrefPage::customIPChecked(bool on)
{
	gp->custom_ip->setEnabled(on);
	gp->custom_ip_label->setEnabled(on);
}

void GeneralPrefPage::dhtChecked(bool on)
{
	gp->dht_port->setEnabled(on);
	gp->dht_port_label->setEnabled(on);
}

void GeneralPrefPage::updateData()
{
	gp->show_systray_icon->setChecked(Settings::showSystemTrayIcon());
	gp->show_speedbar->setChecked(Settings::showSpeedBarInTrayIcon());
	gp->downloadBandwidth->setValue(Settings::downloadBandwidth());
	gp->uploadBandwidth->setValue(Settings::uploadBandwidth());
	gp->show_popups->setChecked(Settings::showPopups());
	KURLRequester* u = gp->temp_dir;
	u->fileDialog()->setMode(KFile::Directory);

	if (Settings::tempDir() == QString::null)
	{
		QString data_dir = KGlobal::dirs()->saveLocation("data", "ktorrent");

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

	
	//completed dir
	u = gp->urlCompletedDir;
	u->fileDialog()->setMode(KFile::Directory);
	bool useCompletedDir = Settings::useCompletedDir();
	QString completedDir = Settings::completedDir();
	gp->checkCompletedDir->setChecked(useCompletedDir);
	u->setEnabled(useCompletedDir);	
	u->setURL(!completedDir.isEmpty() ? completedDir : QDir::homeDirPath());
	
	//copy .torrent dir
	u = gp->urlTorrentDir;
	u->fileDialog()->setMode(KFile::Directory);
	bool useTorrentDir = Settings::useTorrentCopyDir();
	QString torrentDir = Settings::torrentCopyDir();	
	gp->checkTorrentDir->setChecked(useTorrentDir);
	u->setEnabled(useTorrentDir);
	u->setURL(!torrentDir.isEmpty() ? torrentDir : QDir::homeDirPath());
	

	gp->custom_ip->setText(Settings::externalIP());

	bool useExternalIP = Settings::useExternalIP();
	gp->custom_ip_check->setChecked(useExternalIP);
	gp->custom_ip->setEnabled(useExternalIP);
	gp->custom_ip_label->setEnabled(useExternalIP);

	gp->use_dht->setChecked(Settings::dhtSupport());
	gp->dht_port->setValue(Settings::dhtPort());
	gp->dht_port->setEnabled(Settings::dhtSupport());
	gp->dht_port_label->setEnabled(Settings::dhtSupport());

	gp->use_encryption->setChecked(Settings::useEncryption());
	gp->allow_unencrypted->setChecked(Settings::allowUnencryptedConnections());
	gp->allow_unencrypted->setEnabled(Settings::useEncryption());
}

void GeneralPrefPage::deleteWidget()
{
	delete gp;
	gp = 0;
}

/////////////////////////////////

AdvancedPrefPage::AdvancedPrefPage() :
		kt::PrefPageInterface(i18n("Advanced"), i18n("Advanced Options"),
							  KGlobal::iconLoader()->loadIcon("package_settings", KIcon::NoGroup)), ap(0)
{}

AdvancedPrefPage::~AdvancedPrefPage()
{
	delete ap;
}

bool AdvancedPrefPage::apply()
{
	Settings::setMemoryUsage(ap->mem_usage->currentItem());
	Settings::setGuiUpdateInterval(ap->gui_interval->currentItem());
	Settings::setDSCP(ap->dscp->value());
	Settings::setAllwaysDoUploadDataCheck(!ap->no_recheck->isChecked());
	Settings::setMaxSizeForUploadDataCheck(ap->recheck_size->value());
	Settings::setAutoRecheck(ap->auto_recheck->isChecked());
	Settings::setMaxCorruptedBeforeRecheck(ap->num_corrupted->value());
	Settings::setDoNotUseKDEProxy(ap->do_not_use_kde_proxy->isChecked());
	Settings::setHttpTrackerProxy(ap->http_proxy->text());
	Settings::setEta(ap->eta->currentItem());
	Settings::setFullDiskPrealloc(ap->full_prealloc->isChecked());
	Settings::setFullDiskPreallocMethod(ap->full_prealloc_method->currentItem());
	Settings::setCpuUsage(ap->cpu_usage->value());
	Settings::setDiskPrealloc(!ap->prealloc_disabled->isChecked());
	Settings::setMaxConnectingSockets(ap->max_con_setups->value());
	return true;
}

void AdvancedPrefPage::updateData()
{
	ap->mem_usage->setCurrentItem(Settings::memoryUsage());
	ap->gui_interval->setCurrentItem(Settings::guiUpdateInterval());
	ap->dscp->setValue(Settings::dSCP());
	ap->no_recheck->setChecked(!Settings::allwaysDoUploadDataCheck());
	ap->recheck_size->setEnabled(!Settings::allwaysDoUploadDataCheck());
	ap->recheck_size->setValue(Settings::maxSizeForUploadDataCheck());
	ap->auto_recheck->setChecked(Settings::autoRecheck());
	ap->num_corrupted->setValue(Settings::maxCorruptedBeforeRecheck());
	ap->num_corrupted->setEnabled(Settings::autoRecheck());
	ap->do_not_use_kde_proxy->setChecked(Settings::doNotUseKDEProxy());
	ap->http_proxy->setText(Settings::httpTrackerProxy());
	ap->http_proxy->setEnabled(Settings::doNotUseKDEProxy());
	ap->eta->setCurrentItem(Settings::eta());
	ap->full_prealloc->setChecked(Settings::fullDiskPrealloc());
	ap->full_prealloc_method->setCurrentItem(Settings::fullDiskPreallocMethod());
	ap->cpu_usage->setValue(Settings::cpuUsage());
	ap->prealloc_disabled->setChecked(!Settings::diskPrealloc());
	ap->max_con_setups->setValue(Settings::maxConnectingSockets());
}

void AdvancedPrefPage::createWidget(QWidget* parent)
{
	ap = new AdvancedPref(parent);
	updateData();
	connect(ap->no_recheck, SIGNAL(toggled(bool)),
			this, SLOT(noDataCheckChecked(bool)));
	connect(ap->auto_recheck, SIGNAL(toggled(bool)),
			this, SLOT(autoRecheckChecked(bool)));
	connect(ap->do_not_use_kde_proxy, SIGNAL(toggled(bool)),
			this, SLOT(doNotUseKDEProxyChecked(bool)));
	connect(ap->prealloc_disabled,SIGNAL(toggled(bool)),
			this,SLOT(preallocDisabledChecked(bool)));
	
	preallocDisabledChecked(ap->prealloc_disabled->isChecked());
}

void AdvancedPrefPage::deleteWidget()
{
	delete ap;
	ap = 0;
}

void AdvancedPrefPage::noDataCheckChecked(bool on)
{
	ap->recheck_size->setEnabled(on);
}

void AdvancedPrefPage::autoRecheckChecked(bool on)
{
	ap->num_corrupted->setEnabled(on);
}

void AdvancedPrefPage::doNotUseKDEProxyChecked(bool on)
{
	ap->http_proxy->setEnabled(on);
}

void AdvancedPrefPage::preallocDisabledChecked(bool on)
{
	ap->full_prealloc->setEnabled(!on);
	if (!on && ap->full_prealloc->isChecked())
		ap->full_prealloc_method->setEnabled(true);
	else
		ap->full_prealloc_method->setEnabled(false);
}

#include "pref.moc"
