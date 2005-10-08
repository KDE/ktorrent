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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include <libtorrent/torrentcontrol.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <math.h>
#include "ktorrentviewitem.h"
#include "functions.h"

using namespace bt;

static QString StatusToString(TorrentControl* tc,TorrentControl::Status s)
{
	switch (s)
	{
		case TorrentControl::NOT_STARTED :
			return i18n("Not started");
		case TorrentControl::COMPLETE :
			return i18n("Completed");
		case TorrentControl::SEEDING :
			return i18n("Seeding");
		case TorrentControl::DOWNLOADING:
			return i18n("Downloading");
		case TorrentControl::STALLED:
			return i18n("Stalled");
		case TorrentControl::STOPPED:
			return i18n("Stopped");
		case TorrentControl::ERROR :
			return i18n("Error: ") + tc->getShortErrorMessage(); 
	}
	return QString::null;
}

static QColor StatusToColor(TorrentControl::Status s,const QColorGroup & cg)
{
	QColor green(40,205,40);
	QColor yellow(255,174,0);
	switch (s)
	{
		case TorrentControl::NOT_STARTED :
		case TorrentControl::STOPPED:
			return cg.text();
			
		case TorrentControl::SEEDING :
		case TorrentControl::DOWNLOADING:
		case TorrentControl::COMPLETE :
			return green;
		case TorrentControl::STALLED:
			return yellow;
		case TorrentControl::ERROR :
			return Qt::red;
	}
	return QString::null;
}



KTorrentViewItem::KTorrentViewItem(QListView* parent,bt::TorrentControl* tc)
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

	setText(0,tc->getTorrentName());
	TorrentControl::Status status = tc->getStatus();
	setText(1,StatusToString(tc,status));
	Uint32 nb = tc->getBytesDownloaded() > tc->getTotalBytes() ?
			tc->getTotalBytes() : tc->getBytesDownloaded();
	
	setText(2,BytesToString(nb));
	setText(3,BytesToString(tc->getTotalBytesToDownload()));
	setText(4,BytesToString(tc->getBytesUploaded()));
	if (tc->getBytesLeft() == 0)
		setText(5,KBytesPerSecToString(0));
	else
		setText(5,KBytesPerSecToString(tc->getDownloadRate() / 1024.0));
	setText(6,KBytesPerSecToString(tc->getUploadRate() / 1024.0));
  
	KLocale* loc = KGlobal::locale();
	if (tc->getBytesLeft() == 0)
	{
		setText(7,i18n("finished"));
	}
	else 
	{
		float bytes_downloaded = (float)tc->getBytesDownloaded();
		if( bytes_downloaded < 1 ) //if we just started download use old algorithm
		{
			if (tc->getDownloadRate() == 0)
				setText(7,i18n("infinity"));
			else
			{
				Uint32 secs = (int)floor( (float)tc->getBytesLeft() / (float)tc->getDownloadRate() );
				QTime t;
				t = t.addSecs(secs);
				setText(7,loc->formatTime(t,true,true));
			}
		}
		else 
		{
			double avg_speed = (double)bytes_downloaded / (double)tc->getRunningTimeDL();
			double eta = tc->getBytesLeft() / avg_speed;
			QTime t;
			t = t.addSecs((int)floor(eta));
			setText(7,loc->formatTime(t,true,true));
		}
	}
	
	setText(8,QString::number(tc->getNumPeers()));

	double perc = 0;
	if (tc->getBytesLeft() == 0)
	{
		perc = 100.0;
	}
	else
	{
		perc = 100.0 - ((double)tc->getBytesLeft() / tc->getTotalBytesToDownload()) * 100.0;
		if (perc > 100.0)
			perc = 100.0;
		else if (perc > 99.9)
			perc = 99.9;
	}
	setText(9,i18n("%1 %").arg(loc->formatNumber(perc,2)));
}



int KTorrentViewItem::compare(QListViewItem * i,int col,bool) const
{
	KTorrentViewItem* other = (KTorrentViewItem*)i;
	TorrentControl* otc = other->tc;
	switch (col)
	{
		case 0: return QString::compare(tc->getTorrentName(),otc->getTorrentName());
		case 1: return QString::compare(StatusToString(tc,tc->getStatus()),
										StatusToString(otc,otc->getStatus()));
		case 2: return CompareVal(tc->getBytesDownloaded(),otc->getBytesDownloaded());
		case 3: return CompareVal(tc->getTotalBytesToDownload(),otc->getTotalBytesToDownload());
		case 4: return CompareVal(tc->getBytesUploaded(),otc->getBytesUploaded());
		case 5: return CompareVal(tc->getDownloadRate(),otc->getDownloadRate());
		case 6: return CompareVal(tc->getUploadRate(),otc->getUploadRate());
		case 7: return QString::compare(text(6),other->text(6));
		case 8: return CompareVal(tc->getNumPeers(),otc->getNumPeers());
		case 9:
		{
			double perc = ((double)tc->getBytesDownloaded() / tc->getTotalBytesToDownload()) * 100.0;
			if (perc > 100.0)
				perc = 100.0;
			double operc = ((double)otc->getBytesDownloaded() / otc->getTotalBytesToDownload()) * 100.0;
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
		_cg.setColor(QColorGroup::Text, StatusToColor(tc->getStatus(),cg));


	KListViewItem::paintCell(p,_cg,column,width,align);
}

