/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <kfiledialog.h>
#include <qprogressbar.h>
#include <qdir.h>


#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/waitjob.h>

#include <interfaces/guiinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <torrent/server.h>
#include <peer/authenticationmonitor.h>
#include <groups/groupmanager.h>

#include "settings.h"
#include "pluginmanager.h"
#include "core.h"
#include "fileselectdlg.h"

using namespace bt;

namespace kt
{
	const Uint32 CORE_UPDATE_INTERVAL = 250;

	Core::Core(kt::GUIInterface* gui) : gui(gui),keep_seeding(true)
	{
		UpdateCurrentTime();
		qman = new QueueManager();
		connect(qman, SIGNAL(lowDiskSpace(kt::TorrentInterface*,bool)), this, SLOT(onLowDiskSpace(kt::TorrentInterface*,bool)));

		data_dir = Settings::tempDir().path();
		bool dd_not_exist = !bt::Exists(data_dir);
		if (data_dir == QString::null || dd_not_exist)
		{
			data_dir = kt::DataDir();
			if (dd_not_exist)
			{
				Settings::setTempDir(data_dir);
				Settings::self()->writeConfig();
			}
		}

		removed_bytes_up = removed_bytes_down = 0;

		if (!data_dir.endsWith(bt::DirSeparator()))
			data_dir += bt::DirSeparator();

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
				gui->infoMsg(i18n("Specified port (%1) is unavailable or in"
						" use by another application. KTorrent is now using port %2.")
						 .arg(port).arg(port + i - 1));

			Out(SYS_GEN|LOG_NOTICE) << "Bound to port " << (port + i - 1) << endl;
		}
		else
		{
			gui->errorMsg(i18n("KTorrent is unable to accept connections because the ports %1 to %2 are "
				   "already in use by another program.").arg(port).arg(port + i - 1));
			Out(SYS_GEN|LOG_IMPORTANT) << "Cannot find free port" << endl;
		}

		
		pman = new kt::PluginManager(this,gui);
		gman = new kt::GroupManager();
		applySettings(true);
		gman->loadGroups();
	}
	
	Core::~Core()
	{
		delete qman;
		delete pman;
		delete gman;
	}

	void Core::applySettings()
	{
		applySettings(false);
		settingsChanged();
	}

	void Core::applySettings(bool change_port)
	{
		ApplySettings();
		setMaxDownloads(Settings::maxDownloads());
		setMaxSeeds(Settings::maxSeeds());
				setKeepSeeding(Settings::keepSeeding());
		
		changeDataDir(Settings::tempDir().path());
		if (change_port)
			changePort(Settings::port());
		        
		//update QM
		getQueueManager()->orderQueue();
	}

	void Core::loadPlugins()
	{
		pman->loadConfigFile(kt::DataDir() + "plugins");
		pman->loadPluginList();
	}

	bool Core::init(TorrentControl* tc,bool silently)
	{
		bool user = false;
		bool start_torrent = false;

		connectSignals(tc);
		qman->append(tc);
		if (!silently)
		{
			if (!gui->selectFiles(tc,&user,&start_torrent))
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
			gui->errorMsg(err.toString());
			remove(tc,false);
			return false;
		}
			
		if (tc->hasExistingFiles())
		{
			gui->dataScan(tc,true,true,QString::null);
		}
			
		tc->setPreallocateDiskSpace(true);

		if(Settings::maxRatio()>0)
			tc->setMaxShareRatio(Settings::maxRatio()); 
		
		torrentAdded(tc);
		qman->torrentAdded(tc,user,start_torrent);
		//now copy torrent file to user specified dir if needed
		if(Settings::useTorrentCopyDir())
		{
			QString torFile = tc->getTorDir();
			if(!torFile.endsWith(bt::DirSeparator()))
				torFile += bt::DirSeparator();
			
			torFile += "torrent";
			QString destination = Settings::torrentCopyDir().path();
			if(!destination.endsWith(bt::DirSeparator()))
				destination += bt::DirSeparator();
			
			destination += tc->getStats().torrent_name + ".torrent";
			KIO::file_copy(torFile,destination,-1,false,false,false);
		}
		return true;
	}
	
	bool Core::load(const QByteArray & data,const QString & dir,bool silently, const KUrl& url)
	{
		QString tdir = findNewTorrentDir();
		TorrentControl* tc = 0;
		try
		{
			Out(SYS_GEN|LOG_NOTICE) << "Loading torrent from data " << endl;
			tc = new TorrentControl();
			tc->init(qman, data, tdir, dir, 
					 Settings::useSaveDir() ? Settings::saveDir().path() : QString());
			
			if(!init(tc,silently))
				loadingFinished(url, false, true);
			else
				loadingFinished(url, true, false);
			
			return true;
		}
		catch (bt::Error & err)
		{
			gui->errorMsg(err.toString());
			delete tc;
			tc = 0;
			// delete tdir if necesarry
			if (bt::Exists(tdir))
				bt::Delete(tdir,true);
			
			loadingFinished(url, false, false);
			
			return false;
		}
	}

	bool Core::load(const QString & target,const QString & dir,bool silently)
	{
		QString tdir = findNewTorrentDir();
		TorrentControl* tc = 0;
		try
		{
			Out(SYS_GEN|LOG_NOTICE) << "Loading file " << target << endl;
			tc = new TorrentControl();
			tc->init(qman, target, tdir, dir, 
				 Settings::useSaveDir() ? Settings::saveDir().path() : QString());
			
			init(tc,silently);
			return true;
		}
		catch (bt::Error & err)
		{
			gui->errorMsg(err.toString());
			delete tc;
			tc = 0;
			// delete tdir if necesarry
			if (bt::Exists(tdir))
				bt::Delete(tdir,true);
			return false;
		}
	}

	void Core::downloadFinished(KIO::Job *job)
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
			gui->errorMsg(j);
		}
		else
		{
			// load in the file (target is always local)
			QString dir = Settings::saveDir().path();
			if (!Settings::useSaveDir() ||  dir.isNull())
				dir = QDir::homePath();
		
			if (dir != QString::null && load(j->data(),dir,false, j->url()))
				loadingFinished(j->url(),true,false);
			else
				loadingFinished(j->url(),false,true);
		}
	}

	void Core::load(const KUrl& url)
	{
		if (url.isLocalFile())
		{
			QString path = url.path(); 
			QString dir = Settings::saveDir().path();
			if (!Settings::useSaveDir()  || dir.isNull())
				dir =  QDir::homePath();
		
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

	void Core::downloadFinishedSilently(KIO::Job *job)
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
				custom_save_locations.remove(j);
			}
			else if (!Settings::useSaveDir())
			{
				// incase save dir is not set, use home director
				Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << j->url() << " silently, default save location not set !" << endl;
				Out(SYS_GEN|LOG_NOTICE) << "Using home directory instead !" << endl;
				dir = QDir::homePath();
			}
			else
			{
				dir = Settings::saveDir().path();
			}
			
			
			if (dir != QString::null && load(j->data(),dir,true,j->url()))
				loadingFinished(j->url(),true,false);
			else
				loadingFinished(j->url(),false,false);
		}
	}

	void Core::loadSilently(const KUrl& url)
	{
		if (url.isLocalFile())
		{
			QString path = url.path(); 
			QString dir = Settings::saveDir().path();
			if (!Settings::useSaveDir())
			{
				Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << path << " silently, default save location not set !" << endl;
				Out(SYS_GEN|LOG_NOTICE) << "Using home directory instead !" << endl;
				dir = QDir::homePath();
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

	void Core::loadSilentlyDir(const KUrl& url, const KUrl& savedir)
	{
		if (url.isLocalFile())
		{
			QString path = url.path(); 
			QString dir = savedir.path();
			QFileInfo fi(dir);
			if (!fi.exists() || !fi.isWritable() || !fi.isDir())
			{
				Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << path << " silently, destination directory is not OK ! Using default save directory." << endl;
				dir = Settings::saveDir().path();
				if (!Settings::useSaveDir())
				{
					Out(SYS_GEN|LOG_NOTICE) << "Default save directory not set, using home directory !" << endl;
					dir = QDir::homePath();
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

	void Core::start(kt::TorrentInterface* tc)
	{
		kt::TorrentStartResponse reason = qman->start(tc);
		if (reason == NOT_ENOUGH_DISKSPACE || reason == QM_LIMITS_REACHED)
			canNotStart(tc,reason);
	}

	void Core::stop(TorrentInterface* tc, bool user)
	{
		qman->stop(tc, user);
	}

	QString Core::findNewTorrentDir() const
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

	void Core::loadExistingTorrent(const QString & tor_dir)
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
				 Settings::useSaveDir() ? Settings::saveDir().path() : QString());
				
			qman->append(tc);
			connectSignals(tc);
			if (tc->getStats().autostart && tc->getStats().user_controlled)
				start(tc);
			torrentAdded(tc);
		}
		catch (bt::Error & err)
		{
			gui->errorMsg(err.toString());
			delete tc;
		}
	}

	void Core::loadTorrents()
	{
		QDir dir(data_dir);
		QStringList filters;
		filters << "tor*";
		QStringList sl = dir.entryList(filters,QDir::Dirs);
		for (Uint32 i = 0;i < sl.count();i++)
		{
			QString idir = data_dir + sl.at(i);
			if (!idir.endsWith(DirSeparator()))
				idir.append(DirSeparator());

			Out(SYS_GEN|LOG_NOTICE) << "Loading " << idir << endl;
			loadExistingTorrent(idir);
		}
		qman->orderQueue();
	}

	void Core::remove(TorrentInterface* tc,bool data_to)
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
				gui->errorMsg(e.toString());
			}
			
			torrentRemoved(tc);
			qman->torrentRemoved(tc);
			gman->torrentRemoved(tc);

			bt::Delete(dir,false);
		}
		catch (Error & e)
		{
			gui->errorMsg(e.toString());
		}
	}

	void Core::setMaxDownloads(int max)
	{
		qman->setMaxDownloads(max);
	}

	void Core::setMaxSeeds(int max)
	{
		qman->setMaxSeeds(max);
	}

	void Core::torrentFinished(kt::TorrentInterface* tc)
	{
		if (!keep_seeding)
			tc->stop(false);

		finished(tc);
		qman->torrentFinished(tc);
	}

	void Core::setKeepSeeding(bool ks)
	{
		keep_seeding = ks;
		qman->setKeepSeeding(ks);
	}

	void Core::onExit()
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
		{
			WaitJob::execute(job);
		}
		else
			delete job;
		
		qman->clear();
		pman->unloadAll(false);
	}

	bool Core::changeDataDir(const QString & new_dir)
	{
		try
		{
			update_timer.stop();
			// do nothing if new and old dir are the same
			if (KUrl(data_dir) == KUrl(new_dir) || data_dir == (new_dir + bt::DirSeparator()))
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
			
			QList<kt::TorrentInterface*> succes;

			QList<kt::TorrentInterface *>::iterator i = qman->begin();
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

	void Core::rollback(const QList<kt::TorrentInterface*> & succes)
	{
		Out() << "Error, rolling back" << endl;
		update_timer.stop();
		QList<kt::TorrentInterface*>::const_iterator i = succes.begin();
		while (i != succes.end())
		{
			(*i)->rollback();
			i++;
		}
		update_timer.start(CORE_UPDATE_INTERVAL);
	}

	void Core::startAll(int type)
	{
		qman->startall(type);
	}

	void Core::stopAll(int type)
	{
		qman->stopall(type);
	}

	void Core::update()
	{
		bt::UpdateCurrentTime();
		AuthenticationMonitor::instance().update();
		
		QList<kt::TorrentInterface *>::iterator i = qman->begin();
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
				tc->update();
			}
			i++;
		}
	}

	void Core::makeTorrent(const QString & file,const QStringList & trackers,
				int chunk_size,const QString & name,
				const QString & comments,bool seed,
				const QString & output_file,bool priv_tor,QProgressBar* prog, bool decentralized)
	{
#if 0
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
			gui->errorMsg(i18n("Cannot create torrent: %1").arg(e.toString()));
		}
#endif
	}

	CurrentStats Core::getStats()
	{
		CurrentStats stats;
		Uint64 bytes_dl = 0, bytes_ul = 0;
		Uint32 speed_dl = 0, speed_ul = 0;


		for ( QList<kt::TorrentInterface *>::iterator i = qman->begin(); i != qman->end(); ++i )
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

	bool Core::changePort(Uint16 port)
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

	void Core::slotStoppedByError(kt::TorrentInterface* tc, QString msg)
	{
		emit torrentStoppedByError(tc, msg);
	}

	Uint32 Core::getNumTorrentsRunning() const
	{
		return qman->getNumRunning();
	}

	Uint32 Core::getNumTorrentsNotRunning() const
	{
		return qman->count() - qman->getNumRunning();
	}

	bt::QueueManager* Core::getQueueManager()
	{
		return this->qman;
	}

	void Core::torrentSeedAutoStopped( kt::TorrentInterface * tc, kt::AutoStopReason reason)
	{
		qman->startNext();
		if (reason ==  kt::MAX_RATIO_REACHED)
			maxShareRatioReached(tc);
		else
			maxSeedTimeReached(tc);
	}

	void Core::setPausedState(bool pause)
	{
		qman->setPausedState(pause);
	}

	void Core::queue(kt::TorrentInterface* tc)
	{
		qman->queue(tc);
	}

	void Core::aboutToBeStarted(kt::TorrentInterface* tc,bool & ret)
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
					gui->errorMsg(i18n("Cannot deselect missing files: %1").arg(e.toString()));
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
			int ret = KMessageBox::warningYesNo(0,msg, i18n("Recreate"), KStandardGuiItem::cancel());
			if (ret == KMessageBox::Yes)
			{
				try
				{
					tc->recreateMissingFiles();
				}
				catch (bt::Error & e)
				{
					gui->errorMsg(i18n("Cannot recreate data file: %1").arg(e.toString()));
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

	void Core::emitCorruptedData(kt::TorrentInterface* tc)
	{
		corruptedData(tc);
		
	}

	void Core::connectSignals(kt::TorrentInterface* tc)
	{
		connect(tc,SIGNAL(finished(kt::TorrentInterface*)),
				this,SLOT(torrentFinished(kt::TorrentInterface* )));
		connect(tc, SIGNAL(stoppedByError(kt::TorrentInterface*, QString )),
				this, SLOT(slotStoppedByError(kt::TorrentInterface*, QString )));
		connect(tc, SIGNAL(seedingAutoStopped( kt::TorrentInterface*,kt::AutoStopReason )),
				this, SLOT(torrentSeedAutoStopped( kt::TorrentInterface*,kt::AutoStopReason )));
		connect(tc,SIGNAL(aboutToBeStarted( kt::TorrentInterface*,bool & )),
				this, SLOT(aboutToBeStarted( kt::TorrentInterface*,bool & )));
		connect(tc,SIGNAL(corruptedDataFound( kt::TorrentInterface* )),
				this, SLOT(emitCorruptedData( kt::TorrentInterface* )));
		connect(qman, SIGNAL(queuingNotPossible( kt::TorrentInterface* )),
				this, SLOT(enqueueTorrentOverMaxRatio( kt::TorrentInterface* )));
		connect(qman, SIGNAL(lowDiskSpace(kt::TorrentInterface*, bool)),
				this, SLOT(onLowDiskSpace(kt::TorrentInterface*, bool)));
		connect(tc, SIGNAL(needDataCheck(kt::TorrentInterface*)),
				this, SLOT(autoCheckData(kt::TorrentInterface*)));
	}

	float Core::getGlobalMaxShareRatio() const
	{
		return Settings::maxRatio();
	}

	void Core::enqueueTorrentOverMaxRatio(kt::TorrentInterface* tc)
	{
		emit queuingNotPossible(tc);
	}

	void Core::autoCheckData(kt::TorrentInterface* tc)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Doing an automatic data check on " 
					<< tc->getStats().torrent_name << endl;
		
		gui->dataScan(tc,false,true,QString::null);
	}

	void Core::doDataCheck(kt::TorrentInterface* tc)
	{
		bool dummy = false;
		if (tc->isCheckingData(dummy))
			return;
		
		gui->dataScan(tc,false,false,i18n("Checking Data Integrity"));
	}

	void Core::onLowDiskSpace(kt::TorrentInterface * tc, bool stopped)
	{
		emit lowDiskSpace(tc, stopped);
	}
	
	void Core::updateGuiPlugins()
	{
		pman->updateGuiPlugins();
	}

}

#include "core.moc"

