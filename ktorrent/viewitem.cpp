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

	int ETA(const TorrentStats & s,TorrentInterface* tc)
	{
		if (s.bytes_left_to_download == 0)
		{
			return -1;
		}
		else if (s.running) 
		{
			Uint32 secs = tc->getETA();
			if (secs == -1)
				return -2;
			else
				return secs;
		}
		else
			return -2;
	}

	ViewItem::ViewItem(kt::TorrentInterface* tc,View* parent) : QTreeWidgetItem(parent),tc(tc)
	{
		const TorrentStats & s = tc->getStats();
		// stuff that doesn't change
		setText(0,s.torrent_name);
		update();
	}

	ViewItem::~ViewItem()
	{
	}

	void ViewItem::update(bool init)
	{
		const TorrentStats & s = tc->getStats();
	
		if (init || status != s.status)
		{
			setText(1,tc->statusToString());
			status = s.status;
		}

		if (init || bytes_downloaded != s.bytes_downloaded)
		{
			setText(2,BytesToString(s.bytes_downloaded));
			bytes_downloaded = s.bytes_downloaded;
		}

		if (init || total_bytes_to_download != s.total_bytes_to_download)
		{
			setText(3,BytesToString(s.total_bytes_to_download));
			total_bytes_to_download = s.total_bytes_to_download;
		}

		if (init || bytes_uploaded != s.bytes_uploaded)
		{
			setText(4,BytesToString(s.bytes_uploaded));
			bytes_uploaded = s.bytes_uploaded;
		}

		if (init || download_rate != s.download_rate)
		{
			if (s.download_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
			{
				if (s.bytes_left_to_download == 0)
					setText(5,KBytesPerSecToString(0));
				else
					setText(5,KBytesPerSecToString(s.download_rate / 1024.0));
			}
			else
				setText(5, "");
			download_rate = s.download_rate;
		}

		if (init || upload_rate != s.upload_rate)
		{
			if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
				setText(6,KBytesPerSecToString(s.upload_rate / 1024.0));
			else
				setText(6, "");
			upload_rate = s.upload_rate;
		}
		
		int neta = ETA(s,tc);
		if (init || eta != neta)
		{
			if (neta == -2)
				setText(7,QString("%1").arg(QChar(0x221E)));
			else if (neta > 0)
				setText(7,DurationToString(tc->getETA()));
			else
				setText(7,QString::null);
			eta = neta;
		}

		if (init || seeders_total != s.seeders_total || seeders_connected_to != s.seeders_connected_to)
		{
			setText(8,QString("%1 (%2)").arg(QString::number(s.seeders_connected_to)).arg(QString::number(s.seeders_total)));	
			seeders_connected_to = s.seeders_connected_to;
			seeders_total = s.seeders_total;
		}

		if (init || leechers_total != s.leechers_total || leechers_connected_to != s.leechers_connected_to)
		{
			setText(9,QString("%1 (%2)").arg(QString::number(s.leechers_connected_to)).arg(QString::number(s.leechers_total)));
			leechers_total = s.leechers_total;
			leechers_connected_to = s.leechers_connected_to;
		}

		double perc = Percentage(s);
		if (init || perc != percentage)
		{
			setText(10,i18n("%1 %",KGlobal::locale()->formatNumber(perc,2)));
			percentage = perc;
		}

		float ratio = kt::ShareRatio(s);
		if (init || ratio != share_ratio)
		{
			setText(11,QString("%1").arg(KGlobal::locale()->formatNumber(ratio,2)));
			share_ratio = ratio;
		}

		Uint32 secs = tc->getRunningTimeDL();
		if (init || runtime_dl != secs)
		{
			setText(12,secs > 0 ? DurationToString(secs) : "");
			runtime_dl = secs;
		}

		secs = tc->getRunningTimeUL() - tc->getRunningTimeDL();
		if (init || secs != runtime_ul)
		{
			setText(13,secs > 0 ? DurationToString(secs) : "");
			runtime_ul = secs;
		}
	}
	
	bool ViewItem::operator < (const QTreeWidgetItem & other) const
	{
		const ViewItem & vi = (const ViewItem &) other;
		switch (treeWidget()->sortColumn())
		{
		case 0:
		case 1:
			return QTreeWidgetItem::operator < (other);
		case 2: return bytes_downloaded < vi.bytes_downloaded;
		case 3: return total_bytes_to_download < vi.total_bytes_to_download;
		case 4: return bytes_uploaded < vi.bytes_uploaded;
		case 5: return download_rate < vi.download_rate;
		case 6: return upload_rate < vi.upload_rate;
		case 7: return eta < vi.eta;
		case 8: return seeders_connected_to < vi.seeders_connected_to; 
		case 9: return leechers_connected_to < vi.leechers_connected_to;
		case 10: return percentage < vi.percentage;
		case 11: return share_ratio < vi.share_ratio;
		case 12: return runtime_dl < vi.runtime_dl;
		case 13: return runtime_ul < vi.runtime_ul;
		default:
			return false;
		}
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

