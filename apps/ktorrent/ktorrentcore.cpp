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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
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
#include <kio/job.h>
#include <kio/netaccess.h>
#include <qregexp.h>

#include <util/log.h>
#include <torrent/torrentcontrol.h>
#include <torrent/globals.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/waitjob.h>
#include <torrent/torrentcreator.h>
#include <torrent/server.h>
#include <torrent/authenticationmonitor.h>
#include <util/functions.h>
#include <torrent/ipblocklist.h>
#include <kademlia/dhtbase.h>
#include <torrent/torrentfile.h>

#include "pluginmanager.h"
#include <torrent/queuemanager.h>

#include "ktorrentcore.h"
#include "settings.h"
#include "functions.h"
#include "fileselectdlg.h"
#include "scandialog.h"


#include <util/profiler.h>


using namespace bt;
using namespace kt;

const Uint32 CORE_UPDATE_INTERVAL = 250;



KTorrentCore::KTorrentCore(kt::GUIInterface* gui) : max_downloads(0),keep_seeding(true),pman(0)
{
	UpdateCurrentTime();
	
	qman = new QueueManager();
	connect(qman, SIGNAL(lowDiskSpace(kt::TorrentInterface*,bool)), this, SLOT(onLowDiskSpace(kt::TorrentInterface*,bool)));
	
	
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
	update_timer.start(CORE_UPDATE_INTERVAL);
	
	Uint16 port = Settings::port();
	if (port == 0)
	{
		port = 6881;
		Settings::setPort(6881);
	}
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
			                              " use by another application. KTorrent is now using port %2.")
			                         .arg(port).arg(port + i - 1));

		Out(SYS_GEN|LOG_NOTICE) << "Bound to port " << (port + i - 1) << endl;
	}
	else
	{
		KMessageBox::error(0,
			i18n("KTorrent is unable to accept connections because the ports %1 to %2 are "
			   "already in use by another program.").arg(port).arg(port + i - 1));
		Out(SYS_GEN|LOG_IMPORTANT) << "Cannot find free port" << endl;
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

bool KTorrentCore::init(TorrentControl* tc,bool silently)
{
	connectSignals(tc);
	qman->append(tc);
	
	bool user = false;
	bool start_torrent = false;
	
	if (!silently)
	{
		FileSelectDlg dlg(gman, &user, &start_torrent);

		if (dlg.execute(tc) != QDialog::Accepted)
		{
			remove(tc,false);
			return false;
		}
	}	
	
	try
	{
		tc->createFiles();
	}
	catch (bt::Error & err)
	{
		// if we can't create files, just remove the torrent
		KMessageBox::error(0,err.toString());
		remove(tc,false);
		return false;
	}
		
	if (tc->hasExistingFiles())
	{
		ScanDialog* scan_dlg = new ScanDialog(this,true);
		scan_dlg->show();
		scan_dlg->execute(tc,true);
	}
	
	tc->setPreallocateDiskSpace(true);

	if(Settings::maxRatio()>0)
		tc->setMaxShareRatio(Settings::maxRatio()); 
	
	if (Settings::maxSeedTime() > 0)
		tc->setMaxSeedTime(Settings::maxSeedTime());
	
	torrentAdded(tc);
	qman->torrentAdded(tc, user, start_torrent);
		
	
	//now copy torrent file to user specified dir if needed
	if(Settings::useTorrentCopyDir())
	{
		QString torFile = tc->getTorDir();
		
		if(!torFile.endsWith("/"))
			torFile += "/";
		
		torFile += "torrent";
		
		QString destination = Settings::torrentCopyDir();
		
		if(!destination.endsWith("/"))
			destination += "/";
		
		destination += tc->getStats().torrent_name + ".torrent";
		
		try
		{
			bt::CopyFile(torFile, destination, TRUE);
		}
		catch(bt::Error& err)
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Could not copy torrent file to " << destination << endl;
		}
	}
	
	return true;
}

bool KTorrentCore::load(const QByteArray & data,const QString & dir,bool silently, const KURL& url)
{
	QString tdir = findNewTorrentDir();
	TorrentControl* tc = 0;
	try
	{
		Out(SYS_GEN|LOG_NOTICE) << "Loading torrent from data " << endl;
		tc = new TorrentControl();
		tc->init(qman, data, tdir, dir, 
				 Settings::useSaveDir() ? Settings::saveDir() : QString());
		
		if(!init(tc,silently))
			loadingFinished(url, false, true);
		else
			loadingFinished(url, true, false);
		
		return true;
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,err.toString());
		delete tc;
		tc = 0;
		// delete tdir if necesarry
		if (bt::Exists(tdir))
			bt::Delete(tdir,true);
		
		loadingFinished(url, false, false);
		
		return false;
	}
}

bool KTorrentCore::load(const QString & target,const QString & dir,bool silently)
{
	QString tdir = findNewTorrentDir();
	TorrentControl* tc = 0;
	try
	{
		Out(SYS_GEN|LOG_NOTICE) << "Loading file " << target << endl;
		tc = new TorrentControl();
		tc->init(qman, target, tdir, dir, 
			 Settings::useSaveDir() ? Settings::saveDir() : QString());
		
		init(tc,silently);
		return true;
	}
	catch (bt::Error & err)
	{
		KMessageBox::error(0,err.toString());
		delete tc;
		tc = 0;
		// delete tdir if necesarry
		if (bt::Exists(tdir))
			bt::Delete(tdir,true);
		return false;
	}
}

void KTorrentCore::downloadFinished(KIO::Job *job)
{
	KIO::StoredTransferJob* j = (KIO::StoredTransferJob*)job;
	int err = j->error();
	if (err == KIO::ERR_USER_CANCELED)
	{
		loadingFinished(j->url(),false,true);
		return;
	}
	
	if (err)
	{
		loadingFinished(j->url(),false,false);
		j->showErrorDialog(0);
	}
	else
	{
		// load in the file (target is always local)
		QString dir = Settings::saveDir();
		if (!Settings::useSaveDir() || dir.isNull())
			dir = QDir::homeDirPath();
	
		if (dir != QString::null && load(j->data(),dir,false, j->url()))
			loadingFinished(j->url(),true,false);
		else
			loadingFinished(j->url(),false,true);
	}
}

void KTorrentCore::load(const KURL& url)
{
	if (url.isLocalFile())
	{
		QString path = url.path(); 
		QString dir = Settings::saveDir();
		if (!Settings::useSaveDir() || dir.isNull())
			dir = QDir::homeDirPath();
	
		if (dir != QString::null && load(path,dir,false))
			loadingFinished(url,true,false);
		else
			loadingFinished(url,false,true);
	}
	else
	{
		KIO::Job* j = KIO::storedGet(url,false,true);
		connect(j,SIGNAL(result(KIO::Job*)),this,SLOT(downloadFinished( KIO::Job* )));
	}
}

void KTorrentCore::downloadFinishedSilently(KIO::Job *job)
{
	KIO::StoredTransferJob* j = (KIO::StoredTransferJob*)job;
	int err = j->error();
	if (err == KIO::ERR_USER_CANCELED)
	{
		loadingFinished(j->url(),false,true);
	}
	else if (err)
	{
		loadingFinished(j->url(),false,false);
	}
	else
	{
		QString dir;
		if (custom_save_locations.contains(j))
		{
			// we have a custom save location so save to that
			dir = custom_save_locations[j].path();
			custom_save_locations.erase(j);
		}
		else if (!Settings::useSaveDir())
		{
			// incase save dir is not set, use home director
			Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << j->url() << " silently, default save location not set !" << endl;
			Out(SYS_GEN|LOG_NOTICE) << "Using home directory instead !" << endl;
			dir = QDir::homeDirPath();
		}
		else
		{
			dir = Settings::saveDir();
		}
		
		
		if (dir != QString::null && load(j->data(),dir,true,j->url()))
			loadingFinished(j->url(),true,false);
		else
			loadingFinished(j->url(),false,false);
	}
}

void KTorrentCore::loadSilently(const KURL& url)
{
	if (url.isLocalFile())
	{
		QString path = url.path(); 
		QString dir = Settings::saveDir();
		if (!Settings::useSaveDir() || dir.isNull())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << path << " silently, default save location not set !" << endl;
			Out(SYS_GEN|LOG_NOTICE) << "Using home directory instead !" << endl;
			dir = QDir::homeDirPath();
		}
	
		if (dir != QString::null && load(path,dir,true))
			loadingFinished(url,true,false);
		else
			loadingFinished(url,false,true);
	}
	else
	{
		// download to a random file in tmp
		KIO::Job* j = KIO::storedGet(url,false,true);
		connect(j,SIGNAL(result(KIO::Job*)),this,SLOT(downloadFinishedSilently( KIO::Job* )));
	}
}

void KTorrentCore::loadSilentlyDir(const KURL& url, const KURL& savedir)
{
	if (url.isLocalFile())
	{
		QString path = url.path(); 
		QString dir = savedir.path();
		QFileInfo fi(dir);
		if (!fi.exists() || !fi.isWritable() || !fi.isDir())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << path << " silently, destination directory is not OK ! Using default save directory." << endl;
			dir = Settings::saveDir();
			if (!Settings::useSaveDir())
			{
				Out(SYS_GEN|LOG_NOTICE) << "Default save directory not set, using home directory !" << endl;
				dir = QDir::homeDirPath();
			}
		}
	 	
		if (dir != QString::null && load(path,dir,true))
			loadingFinished(url,true,false);
		else
			loadingFinished(url,false,true);
	}
	else
	{
		// download to a random file in tmp
		KIO::Job* j = KIO::storedGet(url,false,true);
		custom_save_locations.insert(j,savedir); // keep track of save location
		connect(j,SIGNAL(result(KIO::Job*)),this,SLOT(downloadFinishedSilently( KIO::Job* )));
	}
}

void KTorrentCore::start(kt::TorrentInterface* tc)
{
	kt::TorrentStartResponse reason = qman->start(tc);
	switch (reason)
	{
		// we can return, the question to ignore the limits will have informed the user
		case MAX_SHARE_RATIO_REACHED: 
		case START_OK:  // start OK is normal 
		case BUSY_WITH_DATA_CHECK: // checking data, so let the torrent be
		case USER_CANCELED:
			return;
		case NOT_ENOUGH_DISKSPACE:
		case QM_LIMITS_REACHED:
			canNotStart(tc,reason);
			break;
	}
}

void KTorrentCore::stop(TorrentInterface* tc, bool user)
{
	qman->stop(tc, user);
}

int KTorrentCore::getNumRunning(bool onlyDownloads, bool onlySeeds) const
{
	return qman->getNumRunning(onlyDownloads, onlySeeds);
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
	
	if (!bt::Exists(idir + "torrent"))
		return;
	
	try
	{
		tc = new TorrentControl();
		tc->init(qman,idir + "torrent",idir,QString::null,
			 Settings::useSaveDir() ? Settings::saveDir() : QString());
			
		qman->append(tc);
		connectSignals(tc);
		if (tc->getStats().autostart && tc->getStats().user_controlled)
			start(tc);
		torrentAdded(tc);
	}
	catch (bt::Error & err)
	{
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

		Out(SYS_GEN|LOG_NOTICE) << "Loading " << idir << endl;
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
		
		try
		{
			if (data_to)
				tc->deleteDataFiles();
		}
		catch (Error & e)
		{
			KMessageBox::error(0,e.toString());
		}
		
		torrentRemoved(tc);
		qman->torrentRemoved(tc);

		bt::Delete(dir,false);
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
	keep_seeding = ks;
	qman->setKeepSeeding(ks);
}

void KTorrentCore::onExit()
{
	// stop timer to prevent updates during wait
	update_timer.stop();
	// stop all authentications going on
	AuthenticationMonitor::instance().clear();
	// shutdown the server
	Globals::instance().shutdownServer();
	
	WaitJob* job = new WaitJob(5000);
	qman->onExit(job);
	// wait for completion of stopped events
	if (job->needToWait())
		WaitJob::execute(job);
	else
		delete job;
	
	pman->unloadAll(false);
	qman->clear();
}

bool KTorrentCore::changeDataDir(const QString & new_dir)
{
	try
	{
		update_timer.stop();
		// do nothing if new and old dir are the same
		if (KURL(data_dir) == KURL(new_dir) || data_dir == (new_dir + bt::DirSeparator()))
		{
			update_timer.start(CORE_UPDATE_INTERVAL);
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
		
		qman->setPausedState(true);
		
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
				qman->setPausedState(false);
				update_timer.start(CORE_UPDATE_INTERVAL);
				return false;
			}
			else
			{
				succes.append(tc);
			}
			i++;
		}
		data_dir = nd;
		qman->setPausedState(false);
		update_timer.start(CORE_UPDATE_INTERVAL);
		return true;
	}
	catch (bt::Error & e)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Error : " << e.toString() << endl;
		update_timer.start(CORE_UPDATE_INTERVAL);
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
	update_timer.start(CORE_UPDATE_INTERVAL);
}

void KTorrentCore::startAll(int type)
{
	qman->startall(type);
}

void KTorrentCore::stopAll(int type)
{
	qman->stopall(type);
}

void KTorrentCore::update()
{
	bt::UpdateCurrentTime();
	AuthenticationMonitor::instance().update();
	
	QPtrList<kt::TorrentInterface>::iterator i = qman->begin();
	//Uint32 down_speed = 0;
	while (i != qman->end())
	{
		bool finished = false;
		kt::TorrentInterface* tc = *i;
		if (tc->isCheckingData(finished))
		{
			if (finished)
				tc->afterDataCheck();
		}
		else if (tc->getStats().running)
		{
			// see if we need to do a auto data check
			if (Settings::autoRecheck() && tc->getStats().num_corrupted_chunks >= Settings::maxCorruptedBeforeRecheck())
			{
				Out(SYS_GEN|LOG_IMPORTANT) << "Doing an automatic data check on " 
						<< tc->getStats().torrent_name << endl;
		
				ScanDialog* scan_dlg = new ScanDialog(this,false);
				scan_dlg->show();
				scan_dlg->execute(tc,true);
			}
			else
			{
				tc->update();
			}
		}
		i++;
	}
}

void KTorrentCore::makeTorrent(const QString & file,const QStringList & trackers,
                               int chunk_size,const QString & name,
                               const QString & comments,bool seed,
							   const QString & output_file,bool priv_tor,KProgress* prog, bool decentralized)
{
	QString tdir;
	try
	{
		if (chunk_size < 0)
			chunk_size = 256;

		bt::TorrentCreator mktor(file,trackers,chunk_size,name,comments,priv_tor, decentralized);
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
			connectSignals(tc);
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
	if (getNumTorrentsRunning() == 0)
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

int KTorrentCore::countSeeds() const
{
	return qman->countSeeds();
}

int KTorrentCore::countDownloads() const
{
	return qman->countDownloads();
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

void KTorrentCore::torrentSeedAutoStopped( kt::TorrentInterface * tc,kt::AutoStopReason reason )
{
	qman->startNext();
	if (reason == kt::MAX_RATIO_REACHED)
		maxShareRatioReached(tc);
	else
		maxSeedTimeReached(tc);
}

int KTorrentCore::getMaxUploadSpeed()
{
	return Settings::maxUploadRate();
}

int KTorrentCore::getMaxDownloadSpeed()
{
	return Settings::maxDownloadRate();
}

void KTorrentCore::setMaxUploadSpeed(int v)
{
	return Settings::setMaxUploadRate(v);
}

void KTorrentCore::setMaxDownloadSpeed(int v)
{
	return Settings::setMaxDownloadRate(v);
}

void KTorrentCore::setPausedState(bool pause)
{
	qman->setPausedState(pause);
}

void KTorrentCore::queue(kt::TorrentInterface* tc)
{
	qman->queue(tc);
}

TorrentInterface* KTorrentCore::getTorFromNumber(int tornumber)
{
        QString tordir = data_dir + "tor" + QString("%1").arg(tornumber) + "/";
	Out() << "tordir " << tordir << endl;
        QPtrList<TorrentInterface>::iterator i = qman->begin();
        while(i != qman->end())
        {
                TorrentInterface* tc = *i;
                QString td = tc->getTorDir();
                if(td == tordir)
                        return tc;
                i++;
        }
        TorrentInterface* nullinterface = 0;
        return nullinterface;
}

QValueList<int> KTorrentCore::getTorrentNumbers(int type = 3)
{
	QValueList<int> tornums;
        QPtrList<TorrentInterface>::iterator i = qman->begin();
        while(i != qman->end())
        {
                TorrentInterface* tc = *i;
		if((type == 1 && tc->getStats().completed) || 
			(type == 2 && !(tc->getStats().completed)))
		{
			Out() << "Skipping a torrent" << endl;
			i++;
			continue;
		}
                QString td = tc->getTorDir();
		Out() << td << endl;
		td = td.remove(0, td.length() - 6);
		td = td.remove(QRegExp("[^0-9]*"));
		Out() << td << endl;
		tornums.append(td.toInt());
                i++;
        }
	return tornums;
}

Uint32 KTorrentCore::getFileCount(int tornumber)
{
        kt::TorrentInterface* tc = getTorFromNumber(tornumber);
	if(tc)
	        return tc->getNumFiles();
	else
		return 0;
}

QCStringList KTorrentCore::getFileNames(int tornumber)
{
        QCStringList filenames;
        kt::TorrentInterface* tc = getTorFromNumber(tornumber);
        if(!tc || tc->getNumFiles() == 0)
                return filenames;
	for(Uint32 i = 0; i < tc->getNumFiles(); i++)
        {
                QCString filename = tc->getTorrentFile(i).getPath().ascii();
                filenames.append(filename);
        }

        return filenames;
}

QValueList<int> KTorrentCore::getFilePriorities(int tornumber)
{
        QValueList<int> priorities;
        kt::TorrentInterface* tc = getTorFromNumber(tornumber);
        if(!tc || tc->getNumFiles() == 0)
                return priorities;

	for(Uint32 i = 0; i < tc->getNumFiles(); i++)
        {
                bt::Priority priority = tc->getTorrentFile(i).getPriority();
                int newpriority;
                switch(priority)
                {
                        case bt::FIRST_PRIORITY:
                                newpriority = 3;
                                break;
                        case bt::LAST_PRIORITY:
                                newpriority = 1;
                                break;
                        case bt::EXCLUDED:
                                newpriority = 0;
                                break;
                        default:
                                newpriority = 2;
                        break;
                }
                priorities.append(newpriority);
        }
        return priorities;
}

void KTorrentCore::setFilePriority(kt::TorrentInterface* tc, Uint32 index, 
		int priority)
{
        bt::Priority newpriority;
        switch(priority)
        {
                case 3:
                        newpriority = bt::FIRST_PRIORITY;
                        break;
                case 1:
                        newpriority = bt::LAST_PRIORITY;
                        break;
                case 0:
                        newpriority = bt::EXCLUDED;
                        break;
                default:
                        newpriority = bt::NORMAL_PRIORITY;
                        break;
        }

        tc->getTorrentFile(index).setPriority(newpriority);
}

void KTorrentCore::announceByTorNum(int tornumber)
{
        kt::TorrentInterface* tc = getTorFromNumber(tornumber);
	if(tc)
	        tc->updateTracker();
}

void KTorrentCore::aboutToBeStarted(kt::TorrentInterface* tc,bool & ret)
{
	ret = true;
	
	QStringList missing;
	if (!tc->hasMissingFiles(missing))
		return;
	
	
	if (tc->getStats().multi_file_torrent)
	{
		QString msg = i18n("Several data files of the torrent \"%1\" are missing, do you want to recreate them, or do you want to not download them?").arg(tc->getStats().torrent_name);
					
		int ret = KMessageBox::warningYesNoCancelList(0,msg,missing,QString::null,
				KGuiItem(i18n("Recreate")),KGuiItem(i18n("Do Not Download")));
		if (ret == KMessageBox::Yes)
		{
			try
			{
				// recreate them
				tc->recreateMissingFiles();
			}
			catch (bt::Error & e)
			{
				KMessageBox::error(0,i18n("Cannot recreate missing files: %1").arg(e.toString()));
				tc->handleError(i18n("Data files are missing"));
				ret = false;
			}
		}
		else if (ret == KMessageBox::No)
		{
			try
			{
				// mark them as do not download
				tc->dndMissingFiles();
			}
			catch (bt::Error & e)
			{
				KMessageBox::error(0,i18n("Cannot deselect missing files: %1").arg(e.toString()));
				tc->handleError(i18n("Data files are missing"));
				ret = false;
			}
		}
		else
		{
			tc->handleError(i18n("Data files are missing"));
			ret = false;
		}
	}
	else
	{
		QString msg = i18n("The file where the data is saved of the torrent \"%1\" is missing, do you want to recreate it?").arg(tc->getStats().torrent_name);
		int ret = KMessageBox::warningYesNo(0,msg, i18n("Recreate"),KGuiItem(i18n("Recreate")),KGuiItem(i18n("Do Not Recreate")));
		if (ret == KMessageBox::Yes)
		{
			try
			{
				tc->recreateMissingFiles();
			}
			catch (bt::Error & e)
			{
				KMessageBox::error(0,i18n("Cannot recreate data file: %1").arg(e.toString()));
				tc->handleError(i18n("Data file is missing"));
				ret = false;
			}
		}
		else
		{
			tc->handleError(i18n("Data file is missing"));
			ret = false;
		}
	}
	
}

void KTorrentCore::emitCorruptedData(kt::TorrentInterface* tc)
{
	corruptedData(tc);
	
}

void KTorrentCore::connectSignals(kt::TorrentInterface* tc)
{
	connect(tc,SIGNAL(finished(kt::TorrentInterface*)),
			this,SLOT(torrentFinished(kt::TorrentInterface* )));
	connect(tc, SIGNAL(stoppedByError(kt::TorrentInterface*, QString )),
			this, SLOT(slotStoppedByError(kt::TorrentInterface*, QString )));
	connect(tc, SIGNAL(seedingAutoStopped(kt::TorrentInterface*, kt::AutoStopReason)),
			this, SLOT(torrentSeedAutoStopped(kt::TorrentInterface*, kt::AutoStopReason)));
	connect(tc,SIGNAL(aboutToBeStarted( kt::TorrentInterface*,bool & )),
			this, SLOT(aboutToBeStarted( kt::TorrentInterface*,bool & )));
	connect(tc,SIGNAL(corruptedDataFound( kt::TorrentInterface* )),
			this, SLOT(emitCorruptedData( kt::TorrentInterface* )));
	connect(qman, SIGNAL(queuingNotPossible(kt::TorrentInterface*)),
			this, SLOT(enqueueTorrentOverMaxRatio( kt::TorrentInterface* )));
	connect(qman, SIGNAL(lowDiskSpace(kt::TorrentInterface*, bool)),
			this, SLOT(onLowDiskSpace(kt::TorrentInterface*, bool)));
	
}

float KTorrentCore::getGlobalMaxShareRatio() const
{
	return Settings::maxRatio();
}

void KTorrentCore::enqueueTorrentOverMaxRatio(kt::TorrentInterface* tc)
{
	emit queuingNotPossible(tc);
}


void KTorrentCore::doDataCheck(kt::TorrentInterface* tc)
{
	bool dummy = false;
	if (tc->isCheckingData(dummy))
		return;
	
	ScanDialog* scan_dlg = new ScanDialog(this,false);
	scan_dlg->setCaption(i18n("Checking Data Integrity"));
	scan_dlg->show();
	scan_dlg->execute(tc,false);
}

void KTorrentCore::onLowDiskSpace(kt::TorrentInterface * tc, bool stopped)
{
	emit lowDiskSpace(tc, stopped);
}

#include "ktorrentcore.moc"
