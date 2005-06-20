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
#include <klocale.h>
#include <kglobal.h>
#include <libtorrent/torrentcontrol.h>
#include <qdatetime.h>
#include <math.h>
#include "ktorrentviewitem.h"

using namespace bt;

const double TO_MEG = (1024.0 * 1024.0);
const double TO_GIG = (1024.0 * 1024.0 * 1024.0);

static QString BytesToString(Uint32 bytes,int precision = -1)
{
	KLocale* loc = KGlobal::locale();
	if (bytes > 1024 * 1024 * 1024)
		return i18n("%1 GB").arg(loc->formatNumber(bytes / TO_GIG,
								precision < 0 ? 2 : precision));
	else
		return i18n("%1 MB").arg(loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
}

static QString KBytesPerSecToString(double speed,int precision = 1)
{
	KLocale* loc = KGlobal::locale();
	return i18n("%1 KB/sec").arg(loc->formatNumber(speed,precision));
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
	setText(1,tc->getStatus());
	setText(2,BytesToString(tc->getBytesDownloaded()) + " / " + BytesToString(tc->getTotalBytes()));
	setText(3,BytesToString(tc->getBytesUploaded()));
	setText(4,KBytesPerSecToString(tc->getDownloadRate() / 1024.0));
	setText(5,KBytesPerSecToString(tc->getUploadRate() / 1024.0));

	KLocale* loc = KGlobal::locale();
	if (tc->getDownloadRate() != 0)
	{
		Uint32 secs = (int)floor((float)tc->getBytesLeft() / (float)tc->getDownloadRate());
		QTime t;
		t = t.addSecs(secs);
		setText(6,loc->formatTime(t,true,true));
	}
	else if (tc->getBytesLeft() == 0)
	{
		setText(6,i18n("finished"));
	}
	else
	{
		setText(6,i18n("infinity"));
	}
	setText(7,QString::number(tc->getNumPeers()));
	double perc = ((double)tc->getBytesDownloaded() / tc->getTotalBytes()) * 100.0;
	setText(8,i18n("%1 %").arg(loc->formatNumber(perc,2)));

	/*
	setText(8,QString("%1 (%2) / %3")
			.arg(tc->getNumChunksDownloaded())
			.arg(tc->getNumChunksDownloading())
			.arg(tc->getTotalChunks()));*/
}

