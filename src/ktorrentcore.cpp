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
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>

#include <libtorrent/log.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/globals.h>
#include <libtorrent/error.h>

#include "ktorrentcore.h"
#include "settings.h"

using namespace bt;

KTorrentCore::KTorrentCore() : max_downloads(0),keep_seeding(true)
{
	data_dir = Settings::tempDir();
	if (data_dir == QString::null)
		data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
	
	if (!data_dir.endsWith(bt::DirSeparator()))
		data_dir += bt::DirSeparator();
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
		
		bool s = (tc->getBytesLeft() == 0 && keep_seeding) ||
				(tc->getBytesLeft() != 0 &&
				(max_downloads == 0 || getNumRunning() < max_downloads));
		if (s)
		{
			Out() << "Starting download" << endl;
			tc->start();
		}

		torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,
			i18n("An error occurred whilst loading the torrent file. "
				"The most likely cause is that the torrent file is corrupted, "
				"or it is not a torrent file at all."),"Error");
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
		
		QString idir = data_dir + *sl.at(i);
		if (!idir.endsWith(DirSeparator()))
			idir.append(DirSeparator());

		Out() << "Loading " << idir << endl;
		
		TorrentControl* tc = 0;
		try
		{
			tc = new TorrentControl();
			tc->init(idir + "torrent",idir);
			downloads.append(tc);
			connect(tc,SIGNAL(finished(bt::TorrentControl*)),
					this,SLOT(torrentFinished(bt::TorrentControl* )));
			
			
			
			Out() << "Starting download" << endl;
			bool s = (tc->getBytesLeft() == 0 && keep_seeding) || 
					(tc->getBytesLeft() != 0 &&
						(max_downloads == 0 || getNumRunning() < max_downloads));
			if (s)
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

bool KTorrentCore::changeDataDir(const QString & new_dir)
{
	// do nothing if new and old dir are the same
	if (KURL(data_dir) == KURL(new_dir))
		return true;
	
	// safety check
	if (!KIO::NetAccess::exists(new_dir,false,0))
	{
		if (!KIO::NetAccess::mkdir(new_dir,0,0755))
			return false;
	}

	// make sure new_dir ends with a /
	QString nd = new_dir;
	if (!nd.endsWith(DirSeparator()))
		nd += DirSeparator();

	Out() << "Switching to datadir " << nd << endl;
	// keep track of all TorrentControl's which have succesfully
	// moved to the new data dir
	QPtrList<bt::TorrentControl> succes;
	
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	while (i != downloads.end())
	{
		bt::TorrentControl* tc = *i;
		if (!tc->changeDataDir(nd))
		{
			// failure time to roll back all the succesfull tc's
			rollback(succes);
			// set back the old data_dir in Settings
			Settings::setTempDir(data_dir);
			Settings::self()->writeConfig();
			return false;
		}
		else
		{
			succes.append(tc);
		}
		i++;
	}
	data_dir = nd;
	return true;
}

void KTorrentCore::rollback(const QPtrList<bt::TorrentControl> & succes)
{
	Out() << "Error, rolling back" << endl;
	QPtrList<bt::TorrentControl> ::const_iterator i = succes.begin();
	while (i != succes.end())
	{
		(*i)->rollback();
		i++;
	}
}

void KTorrentCore::startAll()
{
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	while (i != downloads.end())
	{
		bt::TorrentControl* tc = *i;
		if (!tc->isRunning() && (max_downloads == 0 || getNumRunning() < max_downloads))
			tc->start();
		i++;
	}
}

void KTorrentCore::stopAll()
{
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	while (i != downloads.end())
	{
		bt::TorrentControl* tc = *i;
		if (tc->isRunning())
			tc->stop();
		i++;
	}
}

#include "ktorrentcore.moc"
