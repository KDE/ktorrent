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
								precision < 0 ? 3 : precision));
	else
		return i18n("%1 MB").arg(loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
}

static QString KBytesPerSecToString(double speed,int precision = 3)
{
	KLocale* loc = KGlobal::locale();
	return i18n("%1 kB/sec").arg(loc->formatNumber(speed,precision));
}

KTorrentViewItem::KTorrentViewItem(QListView* parent,bt::TorrentControl* tc)
	: KListViewItem(parent),tc(tc)
{}


KTorrentViewItem::~KTorrentViewItem()
{}

void KTorrentViewItem::update()
{
	setText(0,tc->getTorrentName());
	setText(1,BytesToString(tc->getBytesDownloaded()) + " / " + BytesToString(tc->getTotalBytes()));
	setText(2,BytesToString(tc->getBytesUploaded()));
	setText(3,KBytesPerSecToString(tc->getDownloadRate() / 1024.0));
	setText(4,KBytesPerSecToString(tc->getUploadRate() / 1024.0));

	KLocale* loc = KGlobal::locale();
	if (tc->getDownloadRate() != 0)
	{
		Uint32 secs = (int)floor((float)tc->getBytesLeft() / (float)tc->getDownloadRate());
		QTime t;
		t = t.addSecs(secs);
		setText(5,loc->formatTime(t,true,true));
	}
	else
	{
		setText(5,i18n("infinity"));
	}
	setText(6,QString::number(tc->getNumPeers()));
	setText(7,QString("%1 (%2) / %3")
			.arg(tc->getNumChunksDownloaded())
			.arg(tc->getNumChunksDownloading())
			.arg(tc->getTotalChunks()));
}

