/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#include <unistd.h>
#include <qdir.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kio/netaccess.h>

#include <util/log.h>
#include <torrent/torrentcontrol.h>
#include <torrent/globals.h>
#include <util/error.h>
#include <util/fileops.h>
#include <torrent/torrentcreator.h>
#include <torrent/server.h>
#include <util/functions.h>
#include <torrent/ipblocklist.h>

#include "pluginmanager.h"
#include <torrent/queuemanager.h>

#include "ktorrentcore.h"
#include "settings.h"
#include "functions.h"
#include "fileselectdlg.h"

using namespace bt;
using namespace kt;



KTorrentCore::KTorrentCore(kt::GUIInterface* gui) : max_downloads(0),keep_seeding(true),pman(0)
{
	qman = new QueueManager();
	data_dir = Settings::tempDir();
	bool dd_not_exist = !bt::Exists(data_dir);
	if (data_dir == QString::null || dd_not_exist)
	{
		data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
		if (dd_not_exist)
		{
			Settings::setTempDir(data_dir);
			Settings::writeConfig();
		}
	}

	removed_bytes_up = removed_bytes_down = 0;

	if (!data_dir.endsWith(bt::DirSeparator()))
		data_dir += bt::DirSeparator();
	// 	downloads.setAutoDelete(true);

	connect(&update_timer,SIGNAL(timeout()),this,SLOT(update()));
	update_timer.start(250);

	Uint16 port = Settings::port();
	Uint16 i = 0;
	do
	{
		Globals::instance().initServer(port + i);
		i++;
	}
	while (!Globals::instance().getServer().isOK() && i < 10);

	if (Globals::instance().getServer().isOK())
	{
		if (port != port + i - 1)
			KMessageBox::information(0,
			                         i18n("Specified port (%1) is unavailable or in"
			                              " use by another application. KTorrent is bound to port %2.")
			                         .arg(port).arg(port + i - 1));

		Out() << "Bound to port " << (port + i - 1) << endl;
	}
	else
	{
		KMessageBox::error(0,
		                   i18n("Cannot bind to port %1 or the 10 following ports.").arg(port));
		Out() << "Cannot find free port" << endl;
	}

	
	pman = new kt::PluginManager(this,gui);
}


KTorrentCore::~KTorrentCore()
{
	delete qman;
	delete pman;
}

void KTorrentCore::loadPlugins()
{
	pman->loadConfigFile(KGlobal::dirs()->saveLocation("data","ktorrent") + "plugins");
	pman->loadPluginList();
}

void KTorrentCore::load(const QString & target,const QString & dir)
{
	TorrentControl* tc = 0;
	QString tdir = findNewTorrentDir();
	try
	{
		Out() << "Loading file " << target << endl;
		tc = new TorrentControl();
		tc->init(qman,target,tdir,dir,Settings::saveDir());
		connect(tc,SIGNAL(finished(kt::TorrentInterface*)),
		        this,SLOT(torrentFinished(kt::TorrentInterface* )));
		connect(tc, SIGNAL(stoppedByError(kt::TorrentInterface*, QString )),
		        this, SLOT(slotStoppedByError(kt::TorrentInterface*, QString )));
		// 		downloads.append(tc);
		qman->append(tc);
		if (tc->getStats().multi_file_torrent)
		{
			FileSelectDlg dlg;

			dlg.execute(tc);
		}
		// 		start(tc);
		torrentAdded(tc);
		qman->torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,err.toString());
		delete tc;
		tc = 0;
		// delete tdir if necesarry
		if (bt::Exists(tdir))
			bt::Delete(tdir,true);
	}
}

void KTorrentCore::load(const KURL& url)
{
	QString target;
	// download the contents
	if (KIO::NetAccess::download(url,target,0))
	{
		// load in the file (target is always local)
		QString dir = Settings::saveDir();
		if (dir == QString::null)
			dir = KFileDialog::getExistingDirectory(QString::null, 0,
			                                        i18n("Select Folder to Save To"));


		if (dir != QString::null)
		{
			load(target,dir);
		}
		// and remove the temp file
		KIO::NetAccess::removeTempFile(target);
	}
	else
	{
		KMessageBox::error(0,KIO::NetAccess::lastErrorString(),i18n("Error"));
	}
}

void KTorrentCore::start(kt::TorrentInterface* tc)
{
	qman->start(tc);
}

void KTorrentCore::stop(TorrentInterface* tc, bool user)
{
	qman->stop(tc, user);
}

int KTorrentCore::getNumRunning() const
{
	return qman->getNumRunning(true);
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

void KTorrentCore::loadExistingTorrent(const QString & tor_dir)
{
	TorrentControl* tc = 0;
	
	QString idir = tor_dir;
	if (!idir.endsWith(bt::DirSeparator()))
		idir += bt::DirSeparator();
	
	try
	{
		tc = new TorrentControl();
		tc->init(qman,idir + "torrent",idir,QString::null,Settings::saveDir());
		qman->append(tc);
		connect(tc,SIGNAL(finished(kt::TorrentInterface*)),
				this,SLOT(torrentFinished(kt::TorrentInterface* )));
		connect(tc, SIGNAL(stoppedByError(kt::TorrentInterface*, QString )),
				this, SLOT(slotStoppedByError(kt::TorrentInterface*, QString )));
		if (tc->getStats().autostart)
			start(tc);
		torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
		bt::Delete(idir,true);
		KMessageBox::error(0,err.toString());
		delete tc;
	}
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
		loadExistingTorrent(idir);
	}
	qman->orderQueue();
}

void KTorrentCore::remove(TorrentInterface* tc,bool data_to)
{
	try
	{
		const TorrentStats & s = tc->getStats();
		removed_bytes_up += s.session_bytes_uploaded;
		removed_bytes_down += s.session_bytes_downloaded;
		stop(tc);

		QString dir = tc->getTorDir();
		QString output_path = s.output_path;
		torrentRemoved(tc);
		qman->torrentRemoved(tc);
	
		try
		{
			bt::Delete(dir,false);
		}
		catch (...)
		{
			// if the first delete fails, still try to do the second
			if (data_to)
				bt::Delete(output_path,false);
			throw; // pass the error along
		}
		
		if (data_to)
			bt::Delete(output_path,false);
	}
	catch (Error & e)
	{
		KMessageBox::error(0,e.toString());
	}
}

void KTorrentCore::setMaxDownloads(int max)
{
	qman->setMaxDownloads(max);
}

void KTorrentCore::setMaxSeeds(int max)
{
	qman->setMaxSeeds(max);
}

void KTorrentCore::torrentFinished(kt::TorrentInterface* tc)
{
	if (!keep_seeding)
		tc->stop(false);

	finished(tc);
	qman->torrentFinished(tc);
}

void KTorrentCore::setKeepSeeding(bool ks)
{
	qman->setKeepSeeding(ks);
}

void KTorrentCore::onExit()
{
	// stop timer to prevent updates during wait
	update_timer.stop();
	//pman->saveConfigFile(KGlobal::dirs()->saveLocation("data","ktorrent") + "plugins");
	// 	downloads.clear();
	qman->clear();
	pman->unloadAll(false);
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
		// keep track of all TorrentInterface's which have succesfully
		// moved to the new data dir
		QPtrList<kt::TorrentInterface> succes;

		QPtrList<kt::TorrentInterface>::iterator i = qman->begin();
		while (i != qman->end())
		{
			kt::TorrentInterface* tc = *i;
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

void KTorrentCore::rollback(const QPtrList<kt::TorrentInterface> & succes)
{
	Out() << "Error, rolling back" << endl;
	update_timer.stop();
	QPtrList<kt::TorrentInterface> ::const_iterator i = succes.begin();
	while (i != succes.end())
	{
		(*i)->rollback();
		i++;
	}
	update_timer.start(100);
}

void KTorrentCore::startAll()
{
	qman->startall();
}

void KTorrentCore::stopAll()
{
	qman->stopall();
}

void KTorrentCore::update()
{
	Globals::instance().getServer().update();

	QPtrList<kt::TorrentInterface>::iterator i = qman->begin();
	//Uint32 down_speed = 0;
	while (i != qman->end())
	{
		kt::TorrentInterface* tc = *i;
		if (tc->getStats().running)
		{
			tc->update();
		}
		i++;
	}
}

void KTorrentCore::makeTorrent(const QString & file,const QStringList & trackers,
                               int chunk_size,const QString & name,
                               const QString & comments,bool seed,
							   const QString & output_file,bool priv_tor,KProgress* prog)
{
	QString tdir;
	try
	{
		if (chunk_size < 0)
			chunk_size = 256;

		bt::TorrentCreator mktor(file,trackers,chunk_size,name,comments,priv_tor);
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
		kt::TorrentInterface* tc = mktor.makeTC(tdir);
		if (tc)
		{
			connect(tc,SIGNAL(finished(kt::TorrentInterface*)),
			        this,SLOT(torrentFinished(kt::TorrentInterface* )));
			qman->append(tc);
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

CurrentStats KTorrentCore::getStats()
{
	CurrentStats stats;
	Uint64 bytes_dl = 0, bytes_ul = 0;
	Uint32 speed_dl = 0, speed_ul = 0;


	for ( QPtrList<kt::TorrentInterface>::iterator i = qman->begin(); i != qman->end(); ++i )
	{
		kt::TorrentInterface* tc = *i;
		const TorrentStats & s = tc->getStats();
		speed_dl += s.download_rate;
		speed_ul += s.upload_rate;
		bytes_dl += s.session_bytes_downloaded;
		bytes_ul += s.session_bytes_uploaded;
	}
	stats.download_speed = speed_dl;
	stats.upload_speed = speed_ul;
	stats.bytes_downloaded = bytes_dl + removed_bytes_down;
	stats.bytes_uploaded = bytes_ul + removed_bytes_up;

	return stats;
}

bool KTorrentCore::changePort(Uint16 port)
{
	if (qman->count() == 0)
	{
		Globals::instance().getServer().changePort(port);
		return true;
	}
	else
	{
		return false;
	}
}

void KTorrentCore::slotStoppedByError(kt::TorrentInterface* tc, QString msg)
{
	emit torrentStoppedByError(tc, msg);
}

Uint32 KTorrentCore::getNumTorrentsRunning() const
{
	return qman->getNumRunning();
}

Uint32 KTorrentCore::getNumTorrentsNotRunning() const
{
	return qman->count() - qman->getNumRunning();
}

void KTorrentCore::addBlockedIP(QString& ip)
{
	IPBlocklist& filter = IPBlocklist::instance();
	filter.addRange(ip);
}

void KTorrentCore::removeBlockedIP(QString& ip)
{
	IPBlocklist& filter = IPBlocklist::instance();
	filter.removeRange(ip);
}

bt::QueueManager* KTorrentCore::getQueueManager()
{
	return this->qman;
}

#include "ktorrentcore.moc"
