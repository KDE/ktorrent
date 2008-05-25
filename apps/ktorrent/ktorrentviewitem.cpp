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
#include <kglobal.h>
#include <interfaces/torrentinterface.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <math.h>
#include <interfaces/functions.h>
#include "ktorrentview.h"
#include "ktorrentviewitem.h"

using namespace bt;
using namespace kt;
/*
static QString StatusToString(TorrentInterface* tc,TorrentStatus s)
{
	switch (s)
	{
		case kt::NOT_STARTED :
			return i18n("Not started");
		case kt::COMPLETE :
			return i18n("Completed");
		case kt::SEEDING :
			return i18n("Seeding");
		case kt::DOWNLOADING:
			return i18n("Downloading");
		case kt::STALLED:
			return i18n("Stalled");
		case kt::STOPPED:
			return i18n("Stopped");
		case kt::ERROR :
			return i18n("Error: ") + tc->getShortErrorMessage(); 
		case kt::ALLOCATING_DISKSPACE:
			return i18n("Allocating diskspace");
	}
	return QString::null;
}
*/

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



KTorrentViewItem::KTorrentViewItem(KTorrentView* parent,TorrentInterface* tc)
	: KListViewItem(parent->listView()),tc(tc)
{
	m_parent = parent;
	update();
}


KTorrentViewItem::~KTorrentViewItem()
{}

QCStringList KTorrentViewItem::getTorrentInfo(kt::TorrentInterface* tc)
{
	QCStringList info;
	const TorrentStats & s = tc->getStats();
	info.append(s.torrent_name.local8Bit());
	info.append(tc->statusToString().local8Bit());
	info.append(BytesToString(s.bytes_downloaded).local8Bit());
	info.append(BytesToString(s.total_bytes_to_download).local8Bit());
	info.append(BytesToString(s.bytes_uploaded).local8Bit());
	if (s.bytes_left_to_download == 0)
		info.append(KBytesPerSecToString(0).local8Bit());
	else
		info.append(KBytesPerSecToString(s.download_rate / 1024.0).local8Bit());
	
	info.append(KBytesPerSecToString(s.upload_rate / 1024.0).local8Bit());
	if (s.bytes_left_to_download == 0)
	{
		info.append(QCString(""));
	}
	else if (s.running) 
	{
		Uint32 secs = tc->getETA();
		if(secs == -1)
			info.append(i18n("infinity").local8Bit());
		else
			info.append(DurationToString(secs).local8Bit());
	}
	else
	{
		info.append(i18n("infinity").local8Bit());
	}
	
	info.append(QString::number(s.num_peers).local8Bit());
	info.append(QString(KGlobal::locale()->formatNumber(Percentage(s),2) + " %").local8Bit());
	info.append(KGlobal::locale()->formatNumber(kt::ShareRatio(s),2).local8Bit());
	info.append(QString::number(s.seeders_connected_to).local8Bit());
	info.append(QString::number(s.leechers_connected_to).local8Bit());
	return info;
}

void KTorrentViewItem::update()
{
	const TorrentStats & s = tc->getStats();
	
	if(m_parent->columnVisible(0))
		setText(0,s.torrent_name);
				
	if(m_parent->columnVisible(1))
		setText(1,tc->statusToString());
	
	if(m_parent->columnVisible(2))
	{
		Uint64 nb = /*s.bytes_downloaded > s.total_bytes ? s.total_bytes : */s.bytes_downloaded;
		setText(2,BytesToString(nb));
	}
	
	if(m_parent->columnVisible(3))
		setText(3,BytesToString(s.total_bytes_to_download));
	
	if(m_parent->columnVisible(4))
		setText(4,BytesToString(s.bytes_uploaded));
	
	if(m_parent->columnVisible(5))
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
	}

	if(m_parent->columnVisible(6))
	{
		if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
			setText(6,KBytesPerSecToString(s.upload_rate / 1024.0));
		else
			setText(6, "");
	}

	if(m_parent->columnVisible(7))
	{
		if (s.bytes_left_to_download == 0)
		{
			setText(7,QString::null);
			eta = -1;
		}
		else if (s.running) 
		{
			Uint32 secs = tc->getETA();
			if(secs == -1)
			{
				setText(7,QString("%1").arg(QChar(0x221E)));
				eta = -2;
			}
			else
			{
				eta = secs;
				setText(7,DurationToString(secs));
			}			
		}
		else
		{
			setText(7,QString("%1").arg(QChar(0x221E)));
			eta = -2;
		}
	}
	if(m_parent->columnVisible(8))	
		setText(8,QString::number(s.num_peers));
	
	if(m_parent->columnVisible(8))
	{
		setText(8,QString("%1 (%2)").arg(QString::number(s.seeders_connected_to)).arg(QString::number(s.seeders_total)));	
	}

	if(m_parent->columnVisible(9))
	{
		setText(9,QString("%1 (%2)").arg(QString::number(s.leechers_connected_to)).arg(QString::number(s.leechers_total)));
	}

	if(m_parent->columnVisible(10))
	{
		setText(10,i18n("%1 %").arg(KGlobal::locale()->formatNumber(Percentage(s),2)));
	}
	
	if(m_parent->columnVisible(11))
	{
		float ratio = kt::ShareRatio(s);
		setText(11,QString("%1").arg(KGlobal::locale()->formatNumber(ratio,2)));
	}
	
	if (m_parent->columnVisible(12))
	{
		Uint32 secs = tc->getRunningTimeDL();
		setText(12,secs > 0 ? DurationToString(secs) : "");
	}
	
	if (m_parent->columnVisible(13))
	{
		Uint32 secs = tc->getRunningTimeUL() - tc->getRunningTimeDL();
		setText(13,secs > 0 ? DurationToString(secs) : "");
	}
}



int KTorrentViewItem::compare(QListViewItem * i,int col,bool) const
{
	KTorrentViewItem* other = (KTorrentViewItem*)i;
	TorrentInterface* otc = other->tc;
	const TorrentStats & s = tc->getStats();
	const TorrentStats & os = otc->getStats();
	switch (col)
	{
		case 0: return QString::compare(s.torrent_name,os.torrent_name);
		case 1: return QString::compare(tc->statusToString(),otc->statusToString());
		case 2: return CompareVal(s.bytes_downloaded,os.bytes_downloaded);
		case 3: return CompareVal(s.total_bytes_to_download,os.total_bytes_to_download);
		case 4: return CompareVal(s.bytes_uploaded,os.bytes_uploaded);
		case 5: return CompareVal(s.download_rate,os.download_rate);
		case 6: return CompareVal(s.upload_rate,os.upload_rate);
		case 7: 
			if (eta == other->eta)
				return 0;
			else if (eta >= 0 && other->eta >= 0)
				return CompareVal(eta,other->eta);
			else if (eta == -1) // finsihed is minux one
				return -1;
			else if (other->eta == -1)
				return 1;
			else if (eta == -2) // infinity is minus 2
				return 1;
			else if (other->eta == -2)
				return -1;
			else
				return CompareVal(eta,other->eta);
		case 8: return CompareVal(s.seeders_total,os.seeders_total);
		case 9: return CompareVal(s.leechers_total,os.leechers_total);
		case 10:
		{
			double perc = s.total_bytes_to_download == 0 ? 100.0 : ((double)s.bytes_downloaded / s.total_bytes_to_download) * 100.0;
			if (perc > 100.0)
				perc = 100.0;
			double operc = os.total_bytes_to_download == 0 ? 100.0 : ((double)os.bytes_downloaded / os.total_bytes_to_download) * 100.0;
			if (operc > 100.0)
				operc = 100.0;
			return CompareVal(perc,operc);
		}
		case 11:
		{
			float r1 = kt::ShareRatio(s);
			float r2 = kt::ShareRatio(os);
			return CompareVal(r1,r2);
		}
		case 12:
			return CompareVal(tc->getRunningTimeDL(),otc->getRunningTimeDL());
		case 13:
		{
			Uint32 t = tc->getRunningTimeUL() - tc->getRunningTimeDL();
			Uint32 ot = otc->getRunningTimeUL() - otc->getRunningTimeDL();
			return CompareVal(t,ot);
		}
	}

	return 0;
}

void KTorrentViewItem::paintCell(QPainter* p,const QColorGroup & cg,
								 int column,int width,int align)
{
	QColorGroup _cg( cg );
	QColor c = _cg.text();

	if (column == 1)
		_cg.setColor(QColorGroup::Text, StatusToColor(tc->getStats().status,cg));
	
	if (column == 11)
		_cg.setColor(QColorGroup::Text, ratioToColor(kt::ShareRatio(tc->getStats())));


	KListViewItem::paintCell(p,_cg,column,width,align);
}
