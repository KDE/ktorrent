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
#include <kurl.h>
#include "ktorrentdcop.h"
#include "ktorrent.h"
#include "ktorrentcore.h"
#include "settings.h"

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
	app->load(KURL(file));
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

void KTorrentDCOP::setMaxUploadSpeed(int kbytes_per_sec)
{
	Settings::setMaxUploadRate(kbytes_per_sec);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::setShowSysTrayIcon(bool yes)
{
	Settings::setShowSystemTrayIcon(yes);
	Settings::writeConfig();
	app->applySettings();
}

void KTorrentDCOP::startAll()
{
	app->getCore().startAll();
}

void KTorrentDCOP::stopAll()
{
	app->getCore().stopAll();
}

#include "ktorrentdcop.moc"
		