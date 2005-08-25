/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#include <kpopupmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include "ktorrent.h"
#include "trayicon.h"
#include <qtooltip.h>
#include <kpassivepopup.h>
#include "libtorrent/torrentcontrol.h"
#include "ktorrentcore.h"
#include "functions.h"


using namespace bt;

TrayIcon::TrayIcon( KTorrentCore* tc, QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
	m_core = tc; 
	setPixmap(loadIcon("ktorrent"));
	connect(this,SIGNAL(quitSelected()),kapp,SLOT(quit()));
	connect(m_core, SIGNAL(finished(bt::TorrentControl* )),
			this, SLOT(finished(bt::TorrentControl* )));
	connect(m_core,SIGNAL(torrentStoppedByError(bt::TorrentControl*, QString )),
			this,SLOT(torrentStoppedByError(bt::TorrentControl*, QString )));
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::updateStats(const QString stats)
{
	QToolTip::add(this, "<b>KTorrent</b><br>"+stats);
}

void TrayIcon::finished(TorrentControl* tc) 
{
	double speed_up = 0;
	double speed_down = 0;
	if (tc->getRunningTime() != 0)
	{
		speed_up = double(tc->getBytesUploaded()/1024.0) / (double)tc->getRunningTime();
		speed_down = double(tc->getBytesDownloaded()/1024.0) / (double)tc->getRunningTime();
	}
			
	QString msg = i18n(" has completed downloading.<br>Average speed: %1 DL / %2 UL.")
			.arg(KBytesPerSecToString(speed_down))
			.arg(KBytesPerSecToString(speed_up));
	
	KPassivePopup::message(i18n("Download completed"),
						    tc->getTorrentName()+msg,loadIcon("ktorrent"), this);
}

void TrayIcon::torrentStoppedByError(bt::TorrentControl* tc, QString msg) 
{
	QString err_msg = i18n("%1 has been stopped by following error: <br>%2")
			.arg(tc->getTorrentName()).arg(msg);
	KPassivePopup::message(i18n("Error"),err_msg,loadIcon("ktorrent"),this);
} 
 



#include "trayicon.moc"
