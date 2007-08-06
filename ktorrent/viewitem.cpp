/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include "viewitem.h"
#include "view.h"

namespace kt
{
	static double Percentage(const TorrentStats & s)
	{
		if (s.bytes_left_to_download == 0)
		{
			return 100.0;
		}
		else
		{
			if (s.total_bytes_to_download == 0)
			{
				return 100.0;
			}
			else
			{
				double perc = 100.0 - ((double)s.bytes_left_to_download / s.total_bytes_to_download) * 100.0;
				if (perc > 100.0)
					perc = 100.0;
				else if (perc > 99.9)
					perc = 99.9;
				else if (perc < 0.0)
					perc = 0.0;
				
				return perc;
			}
		}
	}

	ViewItem::ViewItem(kt::TorrentInterface* tc,View* parent) : QTreeWidgetItem(parent),tc(tc)
	{
		update();
	}

	ViewItem::~ViewItem()
	{
	}

	void ViewItem::update()
	{
		const TorrentStats & s = tc->getStats();
		setText(0,s.torrent_name);
		setText(1,tc->statusToString());
		setText(2,BytesToString(s.bytes_downloaded));
		setText(3,BytesToString(s.total_bytes_to_download));
		setText(4,BytesToString(s.bytes_uploaded));

		if (s.download_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
		{
			if (s.bytes_left_to_download == 0)
				setText(5,KBytesPerSecToString(0));
			else
				setText(5,KBytesPerSecToString(s.download_rate / 1024.0));
		}
		else
			setText(5, "");

		if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
			setText(6,KBytesPerSecToString(s.upload_rate / 1024.0));
		else
			setText(6, "");

		if (s.bytes_left_to_download == 0)
		{
			setText(7,QString::null);
		//	eta = -1;
		}
		else if (s.running) 
		{
			Uint32 secs = tc->getETA();
			if(secs == -1)
			{
				setText(7,QString("%1").arg(QChar(0x221E)));
			//	eta = -2;
			}
			else
			{
			//	eta = secs;
				setText(7,DurationToString(secs));
			}			
		}
		else
		{
			setText(7,QString("%1").arg(QChar(0x221E)));
		//	eta = -2;
		}

		setText(8,QString("%1 (%2)").arg(QString::number(s.seeders_connected_to)).arg(QString::number(s.seeders_total)));	
		setText(9,QString("%1 (%2)").arg(QString::number(s.leechers_connected_to)).arg(QString::number(s.leechers_total)));
		setText(10,i18n("%1 %",KGlobal::locale()->formatNumber(Percentage(s),2)));
		float ratio = kt::ShareRatio(s);
		setText(11,QString("%1").arg(KGlobal::locale()->formatNumber(ratio,2)));
		Uint32 secs = tc->getRunningTimeDL();
		setText(12,secs > 0 ? DurationToString(secs) : "");
		secs = tc->getRunningTimeUL() - tc->getRunningTimeDL();
		setText(13,secs > 0 ? DurationToString(secs) : "");
	}

#if 0
	static QColor StatusToColor(TorrentStatus s,const QColorGroup & cg)
	{
		QColor green(40,205,40);
		QColor yellow(255,174,0);
		switch (s)
		{
			case kt::SEEDING :
			case kt::DOWNLOADING:
			case kt::ALLOCATING_DISKSPACE :
				return green;
			case kt::STALLED:
			case kt::CHECKING_DATA:
				return yellow;
			case kt::ERROR :
				return Qt::red;
			case kt::NOT_STARTED :
			case kt::STOPPED:
			case kt::QUEUED:
			case kt::DOWNLOAD_COMPLETE :
			case kt::SEEDING_COMPLETE :
			default:
				return cg.text();
		}
		return cg.text();
	}

	static QColor ratioToColor(float ratio)
	{
		QColor green(40,205,40);
		return ratio > 0.8 ? green : Qt::red;
	}
#endif

}

