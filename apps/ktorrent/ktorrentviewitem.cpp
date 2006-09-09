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
		case kt::COMPLETE :
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
		default:
			return cg.text();
	}
	return cg.text();
}



KTorrentViewItem::KTorrentViewItem(QListView* parent,TorrentInterface* tc)
	: KListViewItem(parent),tc(tc)
{
	update();
}


KTorrentViewItem::~KTorrentViewItem()
{}

void KTorrentViewItem::update()
{
	/*
	addColumn(i18n("File"));
	addColumn(i18n("Status"));
	addColumn(i18n("Dowloaded"));
	addColumn(i18n("Uploaded"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Up Speed"));
	addColumn(i18n("Time Left"));
	addColumn(i18n("Peers"));
	addColumn(i18n("% Complete"));
	*/

	const TorrentStats & s = tc->getStats();

	setText(0,s.torrent_name);
	setText(1,tc->statusToString());
	Uint64 nb = s.bytes_downloaded > s.total_bytes ? s.total_bytes : s.bytes_downloaded;
	setText(2,BytesToString(nb));
	setText(3,BytesToString(s.total_bytes_to_download));
	setText(4,BytesToString(s.bytes_uploaded));
	if (s.bytes_left == 0)
		setText(5,KBytesPerSecToString(0));
	else
		setText(5,KBytesPerSecToString(s.download_rate / 1024.0));
	setText(6,KBytesPerSecToString(s.upload_rate / 1024.0));
  
	KLocale* loc = KGlobal::locale();
	if (s.bytes_left == 0)
	{
		setText(7,i18n("finished"));
		eta = -1;
	}
	else if (s.running) 
	{
		float bytes_downloaded = (float)s.bytes_downloaded;
		if( bytes_downloaded < 1 ) //if we just started download use old algorithm
		{
			if (s.download_rate == 0)
			{
				setText(7,i18n("infinity"));
				eta = -2;
			}
			else
			{
				Uint32 secs = (int)floor( (float)s.bytes_left / (float)s.download_rate);
				eta = secs;
				setText(7,DurationToString(secs));
			}
		}
		else 
		{
			double avg_speed = (double)bytes_downloaded / (double)tc->getRunningTimeDL();
			eta = (Int64)floor(s.bytes_left / avg_speed);
			setText(7,DurationToString((int)floor(s.bytes_left / avg_speed)));
		}
	}
	else
	{
		setText(7,i18n("infinity"));
		eta = -2;
	}
	
	setText(8,QString::number(s.num_peers));

	double perc = 0;
	if (s.bytes_left == 0)
	{
		perc = 100.0;
	}
	else
	{
		if (s.total_bytes_to_download == 0)
		{
			perc = 100.0;
		}
		else
		{
			perc = 100.0 - ((double)s.bytes_left / s.total_bytes_to_download) * 100.0;
			if (perc > 100.0)
				perc = 100.0;
			else if (perc > 99.9)
				perc = 99.9;
			else if (perc < 0.0)
				perc = 0.0;
				
		}
	}
	setText(9,i18n("%1 %").arg(loc->formatNumber(perc,2)));
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
		case 8: return CompareVal(s.num_peers,os.num_peers);
		case 9:
		{
			double perc = s.total_bytes_to_download == 0 ? 100.0 : ((double)s.bytes_downloaded / s.total_bytes_to_download) * 100.0;
			if (perc > 100.0)
				perc = 100.0;
			double operc = os.total_bytes_to_download == 0 ? 100.0 : ((double)os.bytes_downloaded / os.total_bytes_to_download) * 100.0;
			if (operc > 100.0)
				operc = 100.0;
			return CompareVal(perc,operc);
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


	KListViewItem::paintCell(p,_cg,column,width,align);
}

