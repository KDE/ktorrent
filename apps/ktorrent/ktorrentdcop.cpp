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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include <kurl.h>
#include "ktorrentdcop.h"
#include "ktorrent.h"
#include "ktorrentcore.h"
#include "settings.h"
#include <torrent/ipblocklist.h>

KTorrentDCOP::KTorrentDCOP(KTorrent* app)
	: DCOPObject("KTorrent"),app(app)
{}


KTorrentDCOP::~KTorrentDCOP()
{}


bool KTorrentDCOP::changeDataDir(const QString& new_dir)
{
	Settings::setTempDir(new_dir);
	Settings::writeConfig();
	return app->getCore().changeDataDir(new_dir);
}

void KTorrentDCOP::openTorrent(const QString& file)
{
	app->load(KURL::fromPathOrURL(file));
}

void KTorrentDCOP::openTorrentSilently(const QString & file)
{
	app->loadSilently(KURL::fromPathOrURL(file));
}

void KTorrentDCOP::setKeepSeeding(bool ks)
{
	Settings::setKeepSeeding(ks);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setMaxConnectionsPerDownload(int max)
{
	Settings::setMaxConnections(max);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setMaxDownloads(int max)
{
	Settings::setMaxDownloads(max);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setMaxSeeds(int max)
{
	Settings::setMaxSeeds(max);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setMaxUploadSpeed(int kbytes_per_sec)
{
	Settings::setMaxUploadRate(kbytes_per_sec);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setMaxDownloadSpeed(int kbytes_per_sec)
{
	Settings::setMaxDownloadRate(kbytes_per_sec);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setShowSysTrayIcon(bool yes)
{
	Settings::setShowSystemTrayIcon(yes);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::startAll(int type)
{
	app->getCore().startAll(type);
}

void KTorrentDCOP::stopAll(int type)
{
	app->getCore().stopAll(type);
}

void KTorrentDCOP::start(int tornumber)
{
	kt::TorrentInterface* tc = app->getCore().getTorFromNumber(tornumber);
	if(tc)
		app->getCore().start(tc);
}

void KTorrentDCOP::stop(int tornumber, bool user)
{
	kt::TorrentInterface* tc = app->getCore().getTorFromNumber(tornumber);
	if(tc)
		app->getCore().stop(tc, user);
}

QValueList<int> KTorrentDCOP::getTorrentNumbers(int type)
{
	return app->getCore().getTorrentNumbers(type);
}

QCStringList KTorrentDCOP::getTorrentInfo(int tornumber)
{
	QCStringList torrentinfo;
	kt::TorrentInterface* tc = app->getCore().getTorFromNumber(tornumber);
	if(tc)
		torrentinfo = app->getTorrentInfo(tc);
	return torrentinfo;
}


QCStringList KTorrentDCOP::getInfo()
{
	QCStringList info;
	QString thisinfo = app->getStatusInfo();
	info.append(thisinfo.ascii());
	thisinfo = app->getStatusTransfer();
	info.append(thisinfo.ascii());
	thisinfo = app->getStatusSpeed();
	info.append(thisinfo.ascii());
	thisinfo = app->getStatusDHT();
	info.append(thisinfo.ascii());
	return info;
}

int KTorrentDCOP::getFileCount(int tornumber)
{
	return app->getCore().getFileCount(tornumber);
}

QCStringList KTorrentDCOP::getFileNames(int tornumber)
{
	return app->getCore().getFileNames(tornumber);
}

QValueList<int> KTorrentDCOP::getFilePriorities(int tornumber)
{
	return app->getCore().getFilePriorities(tornumber);
}

void KTorrentDCOP::setFilePriority(int tornumber, 
	int index, int priority)
{
	kt::TorrentInterface* tc = app->getCore().getTorFromNumber(tornumber);
	if(tc)
		app->getCore().setFilePriority(tc, index, priority);
}

void KTorrentDCOP::remove(int tornumber, bool del_data)
{
	kt::TorrentInterface* tc = app->getCore().getTorFromNumber(tornumber);
	if(tc)
		app->getCore().remove(tc, del_data);
}

void KTorrentDCOP::announce(int tornumber)
{
	app->getCore().announceByTorNum(tornumber);
}

QCString KTorrentDCOP::dataDir()
{
	QCString dir = Settings::tempDir().ascii();
	return dir;
}

int KTorrentDCOP::maxDownloads()
{
	return Settings::maxDownloads();
}

int KTorrentDCOP::maxSeeds()
{
	return Settings::maxSeeds();
}

int KTorrentDCOP::maxConnections()
{
	return Settings::maxConnections();
}

int KTorrentDCOP::maxUploadRate()
{
	return Settings::maxUploadRate();
}

int KTorrentDCOP::maxDownloadRate()
{
	return Settings::maxDownloadRate();
}

bool KTorrentDCOP::keepSeeding()
{
	return Settings::keepSeeding();
}

bool KTorrentDCOP::showSystemTrayIcon()
{
	return Settings::showSystemTrayIcon();
}

QValueList<int> KTorrentDCOP::intSettings()
{
	QValueList<int> intsettings;
	intsettings.append(Settings::maxDownloads());
	intsettings.append(Settings::maxSeeds());
	intsettings.append(Settings::maxConnections());
	intsettings.append(Settings::maxUploadRate());
	intsettings.append(Settings::maxDownloadRate());
	intsettings.append((int)Settings::keepSeeding());
	intsettings.append((int)Settings::showSystemTrayIcon());
	return intsettings;
}

bool KTorrentDCOP::isBlockedIP(QString ip)
{
	return bt::IPBlocklist::instance().isBlocked(ip);	
}

void KTorrentDCOP::openTorrentSilentlyDir(const QString & file, const QString & savedir)
{
	app->loadSilentlyDir(KURL::fromPathOrURL(file), KURL::fromPathOrURL(savedir));
}

#include "ktorrentdcop.moc"
