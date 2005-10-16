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
#include "interfaces/torrentinterface.h"
#include "ktorrentcore.h"
#include "functions.h"


using namespace bt;
using namespace kt;

TrayIcon::TrayIcon( KTorrentCore* tc, QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
	m_core = tc; 
	setPixmap(loadIcon("ktorrent"));
	connect(this,SIGNAL(quitSelected()),kapp,SLOT(quit()));
	connect(m_core, SIGNAL(finished(kt::TorrentInterface* )),
			this, SLOT(finished(kt::TorrentInterface* )));
	connect(m_core,SIGNAL(torrentStoppedByError(kt::TorrentInterface*, QString )),
			this,SLOT(torrentStoppedByError(kt::TorrentInterface*, QString )));
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::updateStats(const QString stats)
{
	QToolTip::add(this, "<b>KTorrent</b><br>"+stats);
}

void TrayIcon::finished(TorrentInterface* tc) 
{
	double speed_up = 0;
	double speed_down = 0;
	const TorrentStats & s = tc->getStats();
	if (tc->getRunningTimeUL() != 0) //getRunningTimeUL is actually total running time
	{
		speed_up = ((double)s.bytes_uploaded/1024.0) / (double)tc->getRunningTimeUL();
		speed_down = ((double)s.bytes_downloaded/1024.0) / (double)tc->getRunningTimeDL();
	}
			
	QString msg = i18n("<b>%1</b> has completed downloading."
			"<br>Average speed: %2 DL / %3 UL.")
			.arg(s.torrent_name)
			.arg(KBytesPerSecToString(speed_down))
			.arg(KBytesPerSecToString(speed_up));
	
	KPassivePopup::message(i18n("Download completed"),
						   msg,loadIcon("ktorrent"), this);
}

void TrayIcon::torrentStoppedByError(kt::TorrentInterface* tc, QString msg) 
{
	const TorrentStats & s = tc->getStats();
	QString err_msg = i18n("<b>%1</b> has been stopped by the following error: <br>%2")
			.arg(s.torrent_name).arg(msg);
	KPassivePopup::message(i18n("Error"),err_msg,loadIcon("ktorrent"),this);
} 
 



#include "trayicon.moc"
