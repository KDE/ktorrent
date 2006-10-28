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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kpopupmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include "ktorrent.h"
#include "trayicon.h"
#include <qtooltip.h>
#include <kpassivepopup.h>
#include <interfaces/torrentinterface.h>
#include "ktorrentcore.h"
#include <interfaces/functions.h>
#include <net/socketmonitor.h>


using namespace bt;
using namespace kt;

TrayIcon::TrayIcon( KTorrentCore* tc, QWidget *parent, const char *name)
		: KSystemTray(parent, name)
{
	m_core = tc;
	setPixmap(loadIcon("ktorrent"));
	paint=new QPainter( this );
	drawContents ( paint );
	previousDownloadHeight=0;
	previousUploadHeight=0;

	connect(this,SIGNAL(quitSelected()),kapp,SLOT(quit()));
	connect(m_core, SIGNAL(finished(kt::TorrentInterface* )),
	        this, SLOT(finished(kt::TorrentInterface* )));
	connect(m_core,SIGNAL(torrentStoppedByError(kt::TorrentInterface*, QString )),
	        this,SLOT(torrentStoppedByError(kt::TorrentInterface*, QString )));
	connect(m_core,SIGNAL(maxShareRatioReached( kt::TorrentInterface* )),
			this,SLOT(maxShareRatioReached( kt::TorrentInterface* )));
	connect(m_core,SIGNAL(corruptedData( kt::TorrentInterface* )),
			this,SLOT(corruptedData( kt::TorrentInterface* )));
	connect(m_core, SIGNAL(queuingNotPossible( kt::TorrentInterface* )),
			this, SLOT(queuedTorrentOverMaxRatio( kt::TorrentInterface* )));
}

TrayIcon::~TrayIcon()
{}

void TrayIcon::updateStats(const CurrentStats stats, bool showBars,int downloadBandwidth, int uploadBandwidth )
{
	QString tip = i18n("<center><b>KTorrent</b></center><table cellpadding='2' cellspacing='2' align='center'><tr><td><b>Speed:</b></td><td></td></tr><tr><td>Download: <font color='#1c9a1c'>%1</font></td><td>Upload: <font color='#990000'>%2</font></td></tr><tr><td><b>Transfer:</b></td><td></td></tr><tr><td>Download: <font color='#1c9a1c'>%3</font></td><td>Upload: <font color='#990000'>%4</font></td></tr></table>").arg(KBytesPerSecToString((double)stats.download_speed/1024.0)).arg(KBytesPerSecToString((double)stats.upload_speed/1024.0)).arg(BytesToString(stats.bytes_downloaded)).arg(BytesToString(stats.bytes_uploaded));
	QToolTip::add
		(this, tip);
	if(showBars)
		drawSpeedBar(stats.download_speed/1024,stats.upload_speed/1024, downloadBandwidth, uploadBandwidth);
}

void TrayIcon::drawSpeedBar(int downloadSpeed, int uploadSpeed, int downloadBandwidth, int uploadBandwidth )
{
	//check if need repaint
	int DownloadHeight=((downloadSpeed*pixmap()->height())/downloadBandwidth);
	int UploadHeight=((uploadSpeed*pixmap()->height())/uploadBandwidth);
	if(previousDownloadHeight==DownloadHeight && previousUploadHeight==UploadHeight)
		return;
	
	repaint ();

	QBrush brushD(green);
	QBrush brushU(red);
	paint->fillRect(0,pixmap()->height(),2,-DownloadHeight,brushD);
	paint->fillRect(pixmap()->width()-2,pixmap()->height(),2,-UploadHeight,brushU);

	previousDownloadHeight=DownloadHeight;
	previousUploadHeight=UploadHeight;	
}

void TrayIcon::finished(TorrentInterface* tc)
{
	const TorrentStats & s = tc->getStats();
	double speed_up = (double)s.bytes_uploaded / 1024.0;
	double speed_down = (double)(s.bytes_downloaded - s.imported_bytes)/ 1024.0;

	if (Settings::showPopups())
	{
		QString msg = i18n("<b>%1</b> has completed downloading."
						"<br>Average speed: %2 DL / %3 UL.")
						.arg(s.torrent_name)
						.arg(KBytesPerSecToString(speed_down / tc->getRunningTimeDL()))
						.arg(KBytesPerSecToString(speed_up / tc->getRunningTimeUL()));

		KPassivePopup::message(i18n("Download completed"),
							   msg,loadIcon("ktorrent"), this);
	}
}

void TrayIcon::maxShareRatioReached(kt::TorrentInterface* tc)
{
	if (!Settings::showPopups())
		return;
	
	const TorrentStats & s = tc->getStats();
	
	double speed_up = (double)s.bytes_uploaded / 1024.0;
	
	QString msg = i18n("<b>%1</b> has reached its maximum share ratio of %2 and has been stopped."
			"<br>Uploaded %3 at an average speed of %4.")
			.arg(s.torrent_name)
			.arg(s.max_share_ratio)
			.arg(BytesToString(s.bytes_uploaded))
			.arg(KBytesPerSecToString(speed_up / tc->getRunningTimeUL()));
	
	KPassivePopup::message(i18n("Seeding completed"),
						   msg,loadIcon("ktorrent"), this);
}

void TrayIcon::torrentStoppedByError(kt::TorrentInterface* tc, QString msg)
{
	if (!Settings::showPopups())
		return;
	
	const TorrentStats & s = tc->getStats();
	QString err_msg = i18n("<b>%1</b> has been stopped with the following error: <br>%2")
				.arg(s.torrent_name).arg(msg);
	KPassivePopup::message(i18n("Error"),err_msg,loadIcon("ktorrent"),this);
}

void TrayIcon::corruptedData(kt::TorrentInterface* tc)
{
	if (!Settings::showPopups())
		return;
	
	const TorrentStats & s = tc->getStats();
	QString err_msg = i18n("Corrupted data has been found in the torrent <b>%1</b>"
			"<br>It would be a good idea to do a data integrity check on the torrent.")
			.arg(s.torrent_name);
	KPassivePopup::message(i18n("Error"),err_msg,loadIcon("ktorrent"),this);
}

void TrayIcon::queuedTorrentOverMaxRatio(kt::TorrentInterface* tc)
{
	if (!Settings::showPopups())
		return;
	
	const TorrentStats & s = tc->getStats();
	
	double speed_up = (double)s.bytes_uploaded / 1024.0;
	
	QString msg = i18n("<b>%1</b> has reached its maximum share ratio of %2 and cannot be enqueued. Remove the limit manually if you want to continue seeding.").arg(s.torrent_name).arg(s.max_share_ratio);
	
	KPassivePopup::message(i18n("Torrent cannot be enqueued."),
						   msg,loadIcon("ktorrent"), this);
}

SetMaxRate::SetMaxRate( KTorrentCore* tc, int t, QWidget *parent, const char *name):KPopupMenu(parent, name)
{
	m_core = tc;
	type=t;
	makeMenu();
	connect(this,SIGNAL(activated (int)),this,SLOT(rateSelected(int )));
}
void SetMaxRate::makeMenu()
{

	int rate=(type==0) ? m_core->getMaxUploadSpeed() : m_core->getMaxDownloadSpeed();
	int maxBandwidth=(rate > 0) ? rate : (type==0) ? 0 : 20 ;
	int delta = 0;
	int maxBandwidthRounded;

	setCheckable(true);
	insertTitle(i18n("Speed limit in KB/s"));

	if(rate == 0)
		setItemChecked(insertItem(i18n("Unlimited")), true);
	else
		insertItem(i18n("Unlimited"));

	if((maxBandwidth%5)>=3)
		maxBandwidthRounded=maxBandwidth + 5 - (maxBandwidth%5);
	else
		maxBandwidthRounded=maxBandwidth - (maxBandwidth%5);

	for (int i = 0; i < 15; i++)
	{
		QValueList<int> valuePair;
		if (delta == 0)
			valuePair.append(maxBandwidth);
		else
		{
			if((maxBandwidth%5)!=0)
			{
				valuePair.append(maxBandwidthRounded - delta);
				valuePair.append(maxBandwidthRounded + delta);
			}
			else
			{
				valuePair.append(maxBandwidth - delta);
				valuePair.append(maxBandwidth + delta);
			}
		}

		for (int j = 0; j < (int)valuePair.count(); j++)
		{
			if (valuePair[j] >= 1)
			{
				if(rate == valuePair[j] && j==0)
				{
					setItemChecked(insertItem(QString("%1").arg(valuePair[j]),-1, (j == 0) ? 2 : count()), true);
				}
				else
					insertItem(QString("%1").arg(valuePair[j]),-1, (j == 0) ? 2 : count());
			}
		}

		delta += (delta >= 50) ? 50 : (delta >= 20) ? 10 : 5;

	}
}
void SetMaxRate::update()
{
	clear();
	makeMenu();
}

void SetMaxRate::rateSelected(int id)
{
	int rate;
	if(text(id).contains(i18n("Unlimited")))
		rate=0;
	else
		rate=text(id).toInt();
	if(type==0)
	{
		m_core->setMaxUploadSpeed(rate);
		net::SocketMonitor::setUploadCap( Settings::maxUploadRate() * 1024);
	}
	else
	{
		m_core->setMaxDownloadSpeed(rate);
		net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
	}
	Settings::writeConfig();

	update();
}

#include "trayicon.moc"
