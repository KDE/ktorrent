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
#include <qdir.h>
#include <klocale.h>
#include <kglobal.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include <libutil/log.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/globals.h>
#include <libutil/error.h>
#include <libutil/fileops.h>
#include <libtorrent/torrentcreator.h>
//#include <libtorrent/downloadcap.h>
#include <libutil/functions.h>

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

	connect(&update_timer,SIGNAL(timeout()),this,SLOT(update()));
	update_timer.start(100);
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
		start(tc);
		torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,
			i18n("An error occurred whilst loading the torrent file. "
				"The most likely cause is that the torrent file is corrupted, "
				"or it is not a torrent file at all."),i18n("Error"));
		delete tc;
		tc = 0;
	}
}

void KTorrentCore::start(bt::TorrentControl* tc)
{
	bool s = (tc->getBytesLeft() == 0 && keep_seeding) ||
			(tc->getBytesLeft() != 0 &&
			(max_downloads == 0 || getNumRunning() < max_downloads));
	if (s)
	{
		Out() << "Starting download" << endl;
		tc->start();
	}

}

void KTorrentCore::stop(bt::TorrentControl* tc)
{
	if (tc->isStarted() && tc->isRunning())
	{
		tc->stop();
		QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
		while (i != downloads.end())
		{
			TorrentControl* tc = *i;
			start(tc);
			i++;
		}
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
			
			start(tc);
			torrentAdded(tc);
		}
		catch (bt::Error & err)
		{
			bt::Delete(idir,true);
			KMessageBox::error(0,err.toString(),i18n("Error"));
			delete tc;
		}
	}
}

void KTorrentCore::remove(bt::TorrentControl* tc)
{
	stop(tc);
	QString dir = tc->getDataDir();
	torrentRemoved(tc);
	bt::Delete(dir,true);
	downloads.remove(tc);
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
		start(tc);
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
	try
	{
		update_timer.stop();
		// do nothing if new and old dir are the same
		if (KURL(data_dir) == KURL(new_dir) || data_dir == (new_dir + bt::DirSeparator()))
		{
			update_timer.start(100);
			return true;
		}
		
		// safety check
		if (!bt::Exists(new_dir))
			bt::MakeDir(new_dir);
	
	
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
				update_timer.start(100);
				return false;
			}
			else
			{
				succes.append(tc);
			}
			i++;
		}
		data_dir = nd;
		update_timer.start(100);
		return true;
	}
	catch (bt::Error & e)
	{
		Out() << "Error : " << e.toString() << endl;
		update_timer.start(100);
		return false;
	}
}

void KTorrentCore::rollback(const QPtrList<bt::TorrentControl> & succes)
{
	Out() << "Error, rolling back" << endl;
	update_timer.stop();
	QPtrList<bt::TorrentControl> ::const_iterator i = succes.begin();
	while (i != succes.end())
	{
		(*i)->rollback();
		i++;
	}
	update_timer.start(100);
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

void KTorrentCore::update()
{
	QPtrList<bt::TorrentControl>::iterator i = downloads.begin();
	//Uint32 down_speed = 0;
	while (i != downloads.end())
	{
		bt::TorrentControl* tc = *i;
		if (tc->isRunning())
		{
			try
			{
				tc->update();
			//	down_speed += tc->getDownloadRate();
			}
			catch (Error & e)
			{
				tc->stop();
				Out() << "Error " << e.toString() << endl;
				QString errmsg = i18n("The following error occurred: %1\nStopping download ...")
						.arg(e.toString());
				KMessageBox::error(0,errmsg,i18n("Error"));
			}
		}
		i++;
	}

	//bt::DownloadCap::setCurrentSpeed(down_speed);
}

void KTorrentCore::makeTorrent(const QString & file,const QStringList & trackers,
							   int chunk_size,const QString & name,
							   const QString & comments,bool seed,
							   const QString & output_file,KProgress* prog)
{
	QString tdir;
	try
	{
		if (chunk_size < 0)
			chunk_size = 256;

		bt::TorrentCreator mktor(file,trackers,chunk_size,name,comments);
		prog->setTotalSteps(mktor.getNumChunks());
		Uint32 ns = 0;
		while (!mktor.calculateHash())
		{
			prog->setProgress(ns);
			ns++;
			if (ns % 10 == 0)
				KApplication::kApplication()->processEvents();
		}

		mktor.saveTorrent(output_file);
		tdir = findNewTorrentDir();
		bt::TorrentControl* tc = mktor.makeTC(tdir);
		if (tc)
		{
			connect(tc,SIGNAL(finished(bt::TorrentControl*)),
					this,SLOT(torrentFinished(bt::TorrentControl* )));
			downloads.append(tc);
			if (seed)
				start(tc);
			torrentAdded(tc);
		}
	}
	catch (bt::Error & e)
	{
		// cleanup if necessary
		if (bt::Exists(tdir))
			bt::Delete(tdir,true);

		// Show error message
		KMessageBox::error(0,
			i18n("Cannot create torrent: %1").arg(e.toString()),
			i18n("Error"));
	}
}

#include "ktorrentcore.moc"
