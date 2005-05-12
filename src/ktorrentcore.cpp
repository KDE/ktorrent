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
#include <qdir.h>

#include <kglobal.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>

#include <libtorrent/log.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/error.h>

#include "ktorrentcore.h"

using namespace bt;

KTorrentCore::KTorrentCore() : max_downloads(0),keep_seeding(true)
{
	data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
	if (!data_dir.endsWith("/"))
		data_dir += "/";
	downloads.setAutoDelete(true);
}


KTorrentCore::~KTorrentCore()
{}

void KTorrentCore::load(const QString & target)
{
	TorrentControl* tc = 0;
	try
	{
		Out() << "Loading file " << target << endl;
		tc = new TorrentControl();
		tc->init(target,findNewTorrentDir());
		connect(tc,SIGNAL(finished(bt::TorrentControl*)),
				this,SLOT(torrentFinished(bt::TorrentControl* )));
		downloads.append(tc);
		

		if (tc->getBytesLeft() != 0 &&
				  (max_downloads == 0 || getNumRunning() < max_downloads))
		{
			Out() << "Starting download" << endl;
			tc->start();
		}

		torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,err.toString(),"Error");
		delete tc;
		tc = 0;
	}
}

int KTorrentCore::getNumRunning() const
{
	int nr = 0;
	QPtrList<bt::TorrentControl>::const_iterator i = downloads.begin();
	while (i != downloads.end())
	{
		const TorrentControl* tc = *i;
		if (tc->isRunning() && tc->getBytesLeft() != 0)
			nr++;
		i++;
	}
	return nr;
}

QString KTorrentCore::findNewTorrentDir() const
{
	int i = 0;
	while (true)
	{
		QDir d;
		QString dir = data_dir + QString("tor%1/").arg(i);
		if (!d.exists(dir))
		{
			KIO::NetAccess::mkdir(dir,0,0755);
			return dir;
		}
		i++;
	}
	return QString::null;
}

void KTorrentCore::loadTorrents()
{
	QDir dir(data_dir);
	QStringList sl = dir.entryList("tor*",QDir::Dirs);
	for (Uint32 i = 0;i < sl.count();i++)
	{
		Out() << "Loading " << data_dir << *sl.at(i) << endl;
		QString idir = data_dir + *sl.at(i);
		if (!idir.endsWith("/"))
			idir.append("/");
		
		TorrentControl* tc = 0;
		try
		{
			tc = new TorrentControl();
			tc->init(idir + "torrent",idir);
			downloads.append(tc);
			connect(tc,SIGNAL(finished(bt::TorrentControl*)),
					this,SLOT(torrentFinished(bt::TorrentControl* )));
			
			
			
			Out() << "Starting download" << endl;
			if (tc->getBytesLeft() != 0 && 
				(max_downloads == 0 || getNumRunning() < max_downloads))
			{
				tc->start();
			}
			torrentAdded(tc);
		}
		catch (bt::Error & err)
		{
			KIO::NetAccess::del(idir,0);
			KMessageBox::error(0,err.toString(),"Error");
			delete tc;
		}
	}
}

void KTorrentCore::remove(bt::TorrentControl* tc)
{
	tc->stop();
	QString dir = tc->getDataDir();
	torrentRemoved(tc);
	KIO::NetAccess::del(dir,0);
	downloads.remove(tc);
	
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	while (i != downloads.end())
	{
		TorrentControl* tc = *i;
		if (tc->getBytesLeft() != 0 && 
			(max_downloads == 0 || getNumRunning() < max_downloads))
		{
			tc->start();
		}
		i++;
	}
}

void KTorrentCore::setMaxDownloads(int max)
{
	max_downloads = max;
}

void KTorrentCore::torrentFinished(bt::TorrentControl* tc)
{
	if (!keep_seeding)
		tc->stop();
	
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	while (i != downloads.end())
	{
		TorrentControl* tc = *i;
		bool can_start = !tc->isRunning() && !tc->isStarted() && tc->getBytesLeft() != 0;

		if (can_start && (max_downloads == 0 || getNumRunning() < max_downloads))
		{
			tc->start();
		}
		i++;
	}
	finished(tc);
}

void KTorrentCore::setKeepSeeding(bool ks)
{
	keep_seeding = ks;
}

void KTorrentCore::onExit()
{
	downloads.clear();
}

#include "ktorrentcore.moc"
