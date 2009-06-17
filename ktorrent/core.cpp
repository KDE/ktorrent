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
#include <QNetworkInterface>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <qprogressbar.h>
#include <qdir.h>
#include <solid/powermanagement.h>


#include <dbus/dbus.h>
#include <interfaces/guiinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <torrent/torrentcreator.h>
#include <torrent/server.h>
#include <peer/authenticationmonitor.h>

#ifdef GetCurrentTime
#undef GetCurrentTime // on windows this is a define to GetTickCount
#endif 

#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/waitjob.h>

#include <groups/groupmanager.h>
#include <groups/group.h>

#ifdef ENABLE_DHT_SUPPORT
#include <dht/dht.h>
#endif

#include "settings.h"
#include "pluginmanager.h"
#include "core.h"
#include "fileselectdlg.h"
#include "missingfilesdlg.h"
#include "gui.h"
#include "torrentmigratordlg.h"


using namespace bt;

namespace kt
{
	const Uint32 CORE_UPDATE_INTERVAL = 250;

	Core::Core(kt::GUI* gui) : gui(gui),keep_seeding(true),sleep_suppression_cookie(-1)
	{
		UpdateCurrentTime();
		qman = new QueueManager();
		connect(qman, SIGNAL(lowDiskSpace(bt::TorrentInterface*,bool)), 
				this, SLOT(onLowDiskSpace(bt::TorrentInterface*,bool)));
		connect(qman, SIGNAL(queuingNotPossible( bt::TorrentInterface* )),
				this, SLOT(enqueueTorrentOverMaxRatio( bt::TorrentInterface* )));
		connect(qman, SIGNAL(lowDiskSpace(bt::TorrentInterface*, bool)),
				this, SLOT(onLowDiskSpace(bt::TorrentInterface*, bool)));
		
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
		
		Uint16 port = Settings::port();
		if (port == 0)
		{
			port = 6881;
			Settings::setPort(6881);
		}
		
		// Make sure network interface is set properly before server is initialized
		if (Settings::networkInterface() != 0)
		{
			QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
			int iface = Settings::networkInterface();
			if (iface > iface_list.count())
				SetNetworkInterface(QString::null);
			else
				SetNetworkInterface(iface_list[iface - 1].name());
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
						" use by another application. KTorrent is now using port %2.",
						 port,QString::number(port + i - 1)));

			Out(SYS_GEN|LOG_NOTICE) << "Bound to port " << (port + i - 1) << endl;
		}
		else
		{
			gui->errorMsg(i18n("KTorrent is unable to accept connections because the ports %1 to %2 are "
				   "already in use by another program.",port,QString::number(port + i - 1)));
			Out(SYS_GEN|LOG_IMPORTANT) << "Cannot find free port" << endl;
		}

		
		pman = new kt::PluginManager(this,gui);
		gman = new kt::GroupManager();
		applySettings();
		gman->loadGroups();
		connect(qman,SIGNAL(queueOrdered()),this,SLOT(startUpdateTimer()));
		connect(qman,SIGNAL(pauseStateChanged(bool)),gui,SLOT(onPausedStateChanged(bool)));
		
		if (!Settings::oldTorrentsImported()) // check for old torrents if this hasn't happened yet
			QTimer::singleShot(1000,this,SLOT(checkForKDE3Torrents()));
	}
	
	Core::~Core()
	{
		delete qman;
		delete pman;
		delete gman;
	}

	void Core::applySettings()
	{
		ApplySettings();
		setMaxDownloads(Settings::maxDownloads());
		setMaxSeeds(Settings::maxSeeds());
		setKeepSeeding(Settings::keepSeeding());
		
		QString tmp = Settings::tempDir().path();
		if (tmp.isEmpty())
			tmp = kt::DataDir();
		
		changeDataDir(tmp);
		changePort(Settings::port());
		
		//update QM
		getQueueManager()->orderQueue();
		settingsChanged();
	}

	void Core::loadPlugins()
	{
		pman->loadPluginList();
	}

	bool Core::init(TorrentControl* tc,const QString & group,bool silently)
	{
		bool user = false;
		bool start_torrent = false;
		bool skip_check = false;

		if (!silently)
		{
			if (!gui->selectFiles(tc,&user,&start_torrent,group,&skip_check))
			{
				// Cleanup tor dir
				QString dir = tc->getTorDir();
				if (bt::Exists(dir))
					bt::Delete(dir,true);
				delete tc;
				return false;
			}
		}
		else
		{
			// add torrent to group if necessary
			Group* g = gman->find(group);
			if (g)
			{
				g->addTorrent(tc,true);
				gman->saveGroups();
				
				// check if we need to use the default save location of the group
				QString dn = g->groupPolicy().default_save_location;
				if (!dn.isNull() && bt::Exists(dn))
				{
					if (!dn.endsWith(bt::DirSeparator()))
						dn += bt::DirSeparator();
					
					QString ddir = tc->getDataDir();
					if (!ddir.endsWith(bt::DirSeparator()))
						ddir += bt::DirSeparator();

					if (dn != ddir) // only change when really needed
						tc->changeOutputDir(dn, 0);
				}
			}
		}
		
		if (qman->alreadyLoaded(tc->getInfoHash()))
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Torrent " << tc->getDisplayName() << " already loaded" << endl;
			// Cleanup tor dir
			QString dir = tc->getTorDir();
			if (bt::Exists(dir))
				bt::Delete(dir,true);
			delete tc;
			return false;
		}
		
		connectSignals(tc);
		qman->append(tc);
	
		try
		{
			tc->createFiles();
		}
		catch (bt::Error & err)
		{
			if (!silently)
				gui->errorMsg(err.toString());
			Out(SYS_GEN|LOG_IMPORTANT) << err.toString() << endl;
			remove(tc,false);
			return false;
		}
			
		if (tc->hasExistingFiles())
		{
			if (!skip_check)
				gui->dataScan(tc,false,true,QString::null);
			else
				tc->markExistingFilesAsDownloaded();
		}
			
		tc->setPreallocateDiskSpace(true);

		if (Settings::maxRatio() > 0)
			tc->setMaxShareRatio(Settings::maxRatio());
		if (Settings::maxSeedTime() > 0)
			tc->setMaxSeedTime(Settings::maxSeedTime());
		
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
			KIO::file_copy(torFile,destination,-1, KIO::HideProgressInfo);
		}
		return true;
	}
	
	bool Core::load(const QByteArray & data,const QString & dir,const QString & group,bool silently, const KUrl& url)
	{
		QString tdir = findNewTorrentDir();
		TorrentControl* tc = 0;
		try
		{
			Out(SYS_GEN|LOG_NOTICE) << "Loading torrent from data " << endl;
			tc = new TorrentControl();
			tc->init(qman, data, tdir, dir, 
					 Settings::useSaveDir() ? Settings::saveDir().path() : QString());
			tc->setLoadUrl(url);
			
			if(!init(tc,group,silently))
				loadingFinished(url, false, true);
			else
				loadingFinished(url, true, false);
			
			startUpdateTimer();
			return true;
		}
		catch (bt::Error & err)
		{
			if (!silently)
				gui->errorMsg(err.toString());
			else
				canNotLoadSilently(err.toString());
			
			bt::Out(SYS_GEN|LOG_IMPORTANT) << err.toString() << endl;
			
			delete tc;
			tc = 0;
			// delete tdir if necesarry
			if (bt::Exists(tdir))
				bt::Delete(tdir,true);
			
			loadingFinished(url, false, false);
			
			return false;
		}
	}

	bool Core::load(const QString & target,const QString & dir,const QString & group,bool silently)
	{
		QString tdir = findNewTorrentDir();
		TorrentControl* tc = 0;
		try
		{
			Out(SYS_GEN|LOG_NOTICE) << "Loading file " << target << endl;
			tc = new TorrentControl();
			tc->init(qman, target, tdir, dir, 
				 Settings::useSaveDir() ? Settings::saveDir().path() : QString());
			tc->setLoadUrl(KUrl(target));
			
			init(tc,group,silently);
			startUpdateTimer();
			return true;
		}
		catch (bt::Error & err)
		{
			if (!silently)
				gui->errorMsg(err.toString());
			else
				canNotLoadSilently(err.toString());
			
			bt::Out(SYS_GEN|LOG_IMPORTANT) << err.toString() << endl;
			
			delete tc;
			tc = 0;
			// delete tdir if necesarry
			if (bt::Exists(tdir))
				bt::Delete(tdir,true);
			return false;
		}
	}

	void Core::downloadFinished(KJob *job)
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
			
			QString group;
			if (add_to_groups.contains(j))
			{
				group = add_to_groups[j];
				add_to_groups.remove(j);
			}
		
			if (dir != QString::null && load(j->data(),dir,group,false, j->url()))
				loadingFinished(j->url(),true,false);
			else
				loadingFinished(j->url(),false,true);
		}
	}

	void Core::load(const KUrl& url,const QString & group)
	{
		if (url.isLocalFile())
		{
			QString path = url.path(); 
			QString dir = Settings::saveDir().path();
			if (!Settings::useSaveDir()  || dir.isNull())
				dir =  QDir::homePath();
		
			if (dir != QString::null && load(path,dir,group,false))
				loadingFinished(url,true,false);
			else
				loadingFinished(url,false,true);
		}
		else
		{
			KIO::Job* j = KIO::storedGet(url);
			connect(j,SIGNAL(result(KJob*)),this,SLOT(downloadFinished( KJob* )));
			if (!group.isNull())
				add_to_groups.insert(j,group);
		}
	}

	void Core::downloadFinishedSilently(KJob *job)
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
			
			QString group;
			if (add_to_groups.contains(j))
			{
				group = add_to_groups[j];
				add_to_groups.remove(j);
			}
				
			
			if (dir != QString::null && load(j->data(),dir,group,true,j->url()))
				loadingFinished(j->url(),true,false);
			else
				loadingFinished(j->url(),false,false);
		}
	}

	void Core::loadSilently(const KUrl& url,const QString & group)
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
		
			if (dir != QString::null && load(path,dir,group,true))
				loadingFinished(url,true,false);
			else
				loadingFinished(url,false,true);
		}
		else
		{
			// download to a random file in tmp
			KIO::Job* j = KIO::storedGet(url);
			connect(j,SIGNAL(result(KJob*)),this,SLOT(downloadFinishedSilently( KJob* )));
			if (!group.isNull())
				add_to_groups.insert(j,group);
		}
	}
	
	void Core::load(const QByteArray & data,const KUrl& url,const QString & group,const QString & savedir)
	{
		QString dir;
		if (savedir.isEmpty() || !bt::Exists(savedir))
		{
			dir = Settings::saveDir().path();
			if (!Settings::useSaveDir()  || dir.isNull())
				dir =  QDir::homePath();
		}
		else
			dir = savedir;
		
		if (dir != QString::null && load(data,dir,group,false,url))
			loadingFinished(url,true,false);
		else
			loadingFinished(url,false,true);
	}
	
	void Core::loadSilently(const QByteArray & data,const KUrl& url,const QString & group,const QString & savedir)
	{
		QString dir;
		if (savedir.isEmpty() || !bt::Exists(savedir))
		{
			dir = Settings::saveDir().path();
			if (!Settings::useSaveDir())
			{
				Out(SYS_GEN|LOG_NOTICE) << "Cannot load " << url.prettyUrl() << " silently, default save location not set !" << endl;
				Out(SYS_GEN|LOG_NOTICE) << "Using home directory instead !" << endl;
				dir = QDir::homePath();
			}
		}
		else
			dir = savedir;
		
		if (dir != QString::null && load(data,dir,group,true,url))
			loadingFinished(url,true,false);
		else
			loadingFinished(url,false,true);
	}

	/*
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
			
			if (dir != QString::null && load(path,dir,QString(),true))
				loadingFinished(url,true,false);
			else
				loadingFinished(url,false,true);
		}
		else
		{
			// download to a random file in tmp
			KIO::Job* j = KIO::storedGet(url);
			custom_save_locations.insert(j,savedir); // keep track of save location
			connect(j,SIGNAL(result(KJob*)),this,SLOT(downloadFinishedSilently( KJob* )));
		}
	}
*/
	
	void Core::start(bt::TorrentInterface* tc)
	{
		TorrentStartResponse reason = qman->start(tc);
		if (reason == NOT_ENOUGH_DISKSPACE || reason == QM_LIMITS_REACHED)
			canNotStart(tc,reason);
		startUpdateTimer(); // restart update timer
	}
	
	void Core::start(QList<bt::TorrentInterface*> & todo)
	{
		if (todo.isEmpty())
			return;
		
		if (todo.count() == 1)
		{
			start(todo.front());
		}
		else
		{
			qman->start(todo);
			startUpdateTimer(); // restart update timer
		}
	}

	void Core::stop(bt::TorrentInterface* tc, bool user)
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
			if (tc->getStats().autostart && tc->getStats().user_controlled && !tc->overMaxRatio() && !tc->overMaxSeedTime())
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
		for (int i = 0;i < sl.count();i++)
		{
			QString idir = data_dir + sl.at(i);
			if (!idir.endsWith(DirSeparator()))
				idir.append(DirSeparator());

			Out(SYS_GEN|LOG_NOTICE) << "Loading " << idir << endl;
			loadExistingTorrent(idir);
		}
		qman->orderQueue();
	}

	void Core::remove(bt::TorrentInterface* tc,bool data_to)
	{
		try
		{
			const bt::TorrentStats & s = tc->getStats();
			removed_bytes_up += s.session_bytes_uploaded;
			removed_bytes_down += s.session_bytes_downloaded;
			stop(tc,true);

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
			gman->torrentRemoved(tc);
			qman->torrentRemoved(tc);
			gui->updateActions();
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

	void Core::torrentFinished(bt::TorrentInterface* tc)
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
#ifdef ENABLE_DHT_SUPPORT
		// make sure DHT is stopped
		Globals::instance().getDHT().stop();
#endif
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
		
		pman->unloadAll();
		qman->clear();
	}

	bool Core::changeDataDir(const QString & new_dir)
	{
		try
		{
			// do nothing if new and old dir are the same
			if (KUrl(data_dir) == KUrl(new_dir) || data_dir == (new_dir + bt::DirSeparator()))
				return true;

			update_timer.stop();
			// safety check
			if (!bt::Exists(new_dir))
				bt::MakeDir(new_dir);


			// make sure new_dir ends with a /
			QString nd = new_dir;
			if (!nd.endsWith(DirSeparator()))
				nd += DirSeparator();

			Out(SYS_GEN|LOG_DEBUG) << "Switching to datadir " << nd << endl;
			
			qman->setPausedState(true);
			
			QList<bt::TorrentInterface*> succes;

			QList<bt::TorrentInterface *>::iterator i = qman->begin();
			while (i != qman->end())
			{
				bt::TorrentInterface* tc = *i;
				if (!tc->changeTorDir(nd))
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

	void Core::rollback(const QList<bt::TorrentInterface*> & succes)
	{
		Out(SYS_GEN|LOG_DEBUG) << "Error, rolling back" << endl;
		update_timer.stop();
		QList<bt::TorrentInterface*>::const_iterator i = succes.begin();
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
		startUpdateTimer();
	}

	void Core::stopAll(int type)
	{
		qman->stopall(type);
	}
	
	void Core::startUpdateTimer()
	{
		if (!update_timer.isActive())
		{
			Out(SYS_GEN|LOG_DEBUG) << "Started update timer" << endl;
			update_timer.start(CORE_UPDATE_INTERVAL);
			if (Settings::suppressSleep() && sleep_suppression_cookie == -1)
			{
				sleep_suppression_cookie = Solid::PowerManagement::beginSuppressingSleep(i18n("KTorrent is running one or more torrents"));
				if (sleep_suppression_cookie == -1)
				{
					Out(SYS_GEN|LOG_IMPORTANT) << "Failed to suppress sleeping" << endl;
				}
				else
				{
					Out(SYS_GEN|LOG_DEBUG) << "Suppressing sleep" << endl;
				}
			}
		}
	}

	void Core::update()
	{
		try
		{
			bt::UpdateCurrentTime();
			AuthenticationMonitor::instance().update();
			
			QList<bt::TorrentInterface *>::iterator i = qman->begin();
			bool updated = false;
			while (i != qman->end())
			{
				bt::TorrentInterface* tc = *i;
				if (tc->updateNeeded())
				{
					tc->update();
					updated = true;
				}
				i++;
			}
			
			if (!updated)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Stopped update timer" << endl;
				update_timer.stop(); // stop timer when not necessary
				if (sleep_suppression_cookie != -1)
				{
					Solid::PowerManagement::stopSuppressingSleep(sleep_suppression_cookie);
					Out(SYS_GEN|LOG_DEBUG) << "Stopped suppressing sleep" << endl;
					sleep_suppression_cookie = -1;
				}
			}
			else
			{
				// check if the priority of stalled torrents must be decreased
				if (Settings::decreasePriorityOfStalledTorrents())
					qman->checkStalledTorrents(bt::GetCurrentTime(),Settings::stallTimer());
			}
		}
		catch (bt::Error & err)
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Caught bt::Error: " << err.toString() << endl;
		}
	}

	bt::TorrentInterface* Core::makeTorrent(const QString & file,const QStringList & trackers,const KUrl::List & webseeds,
				int chunk_size,const QString & name,
				const QString & comments,bool seed,
				const QString & output_file,bool priv_tor,QProgressBar* prog, bool decentralized)
	{
		QString tdir;
		try
		{
			if (chunk_size < 0)
				chunk_size = 256;

			bt::TorrentCreator mktor(file,trackers,webseeds,chunk_size,name,comments,priv_tor, decentralized);
			prog->setMaximum(mktor.getNumChunks());
			Uint32 ns = 0;
			while (!mktor.calculateHash())
			{
				prog->setValue(ns);
				ns++;
				if (ns % 10 == 0)
					KApplication::kApplication()->processEvents();
			}

			mktor.saveTorrent(output_file);
			tdir = findNewTorrentDir();
			bt::TorrentInterface* tc = mktor.makeTC(tdir);
			if (tc)
			{
				connectSignals(tc);
				qman->append(tc);
				if (seed)
					start(tc);
				torrentAdded(tc);
				return tc;
			}
		}
		catch (bt::Error & e)
		{
			// cleanup if necessary
			if (bt::Exists(tdir))
				bt::Delete(tdir,true);

			// Show error message
			gui->errorMsg(i18n("Cannot create torrent: %1",e.toString()));
		}
		return 0;
	}

	CurrentStats Core::getStats()
	{
		CurrentStats stats;
		Uint64 bytes_dl = 0, bytes_ul = 0;
		Uint32 speed_dl = 0, speed_ul = 0;


		for ( QList<bt::TorrentInterface *>::iterator i = qman->begin(); i != qman->end(); ++i )
		{
			bt::TorrentInterface* tc = *i;
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
		bt::Server & srv = Globals::instance().getServer();
		srv.changePort(port);
		return srv.isOK();
	}

	void Core::slotStoppedByError(bt::TorrentInterface* tc, QString msg)
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

	kt::QueueManager* Core::getQueueManager()
	{
		return this->qman;
	}

	void Core::torrentSeedAutoStopped( bt::TorrentInterface * tc, AutoStopReason reason)
	{
		qman->startNext();
		if (reason ==  MAX_RATIO_REACHED)
			maxShareRatioReached(tc);
		else
			maxSeedTimeReached(tc);
		startUpdateTimer();
	}

	void Core::setPausedState(bool pause)
	{
		qman->setPausedState(pause);
		if (!pause)
			startUpdateTimer();
	}
	
	bool Core::getPausedState()
	{
		return qman->getPausedState();
	}

	void Core::queue(bt::TorrentInterface* tc)
	{
		qman->queue(tc);
	}

	void Core::aboutToBeStarted(bt::TorrentInterface* tc,bool & ret)
	{
		ret = true;
		
		QStringList missing;
		if (!tc->hasMissingFiles(missing))
			return;
		
		
		if (tc->getStats().multi_file_torrent)
		{	
			QString msg = i18n(
					"Several data files of the torrent \"%1\" are missing. \n"
					"Do you want to recreate them, or do you want to not download them?",
					tc->getStats().torrent_name);
			
			MissingFilesDlg dlg(msg,missing,tc,0);
						
			switch (dlg.execute())
			{
				case MissingFilesDlg::CANCEL:
					tc->handleError(i18n("Data files are missing"));
					ret = false;
					break;
				case MissingFilesDlg::DO_NOT_DOWNLOAD:
					try
					{
					// mark them as do not download
						tc->dndMissingFiles();
					}
					catch (bt::Error & e)
					{
						gui->errorMsg(i18n("Cannot deselect missing files: %1",e.toString()));
						tc->handleError(i18n("Data files are missing"));
						ret = false;
					}
					break;
				case MissingFilesDlg::RECREATE:
					try
					{
					// recreate them
						tc->recreateMissingFiles();
					}
					catch (bt::Error & e)
					{
						KMessageBox::error(0,i18n("Cannot recreate missing files: %1",e.toString()));
						tc->handleError(i18n("Data files are missing"));
						ret = false;
					}
					break;
				case MissingFilesDlg::QUIT:
					ret = false;
					QTimer::singleShot(500,kapp,SLOT(quit()));
					break;
				case MissingFilesDlg::NEW_LOCATION_SELECTED:
					ret = true;
					break;
			}
		}
		else
		{
			QString msg = i18n("The file where the data is saved of the torrent \"%1\" is missing. \n"
					"Do you want to recreate it?",tc->getStats().torrent_name);
			MissingFilesDlg dlg(msg,missing,tc,0);
						
			switch (dlg.execute())
			{
				case MissingFilesDlg::CANCEL:
					tc->handleError(i18n("Data file is missing"));
					ret = false;
					break;
				case MissingFilesDlg::RECREATE:
					try
					{
						tc->recreateMissingFiles();
					}
					catch (bt::Error & e)
					{
						gui->errorMsg(i18n("Cannot recreate data file: %1",e.toString()));
						tc->handleError(i18n("Data file is missing"));
						ret = false;
					}
					break;
				case MissingFilesDlg::QUIT:
					ret = false;
					QTimer::singleShot(500,kapp,SLOT(quit()));
					break;
				case MissingFilesDlg::DO_NOT_DOWNLOAD:
					ret = false;
					break;
				case MissingFilesDlg::NEW_LOCATION_SELECTED:
					ret = true;
					break;
			}
		}
		
	}

	void Core::emitCorruptedData(bt::TorrentInterface* tc)
	{
		corruptedData(tc);
		
	}

	void Core::connectSignals(bt::TorrentInterface* tc)
	{
		connect(tc,SIGNAL(finished(bt::TorrentInterface*)),
				this,SLOT(torrentFinished(bt::TorrentInterface* )));
		connect(tc, SIGNAL(stoppedByError(bt::TorrentInterface*, QString )),
				this, SLOT(slotStoppedByError(bt::TorrentInterface*, QString )));
		connect(tc, SIGNAL(seedingAutoStopped( bt::TorrentInterface*,bt::AutoStopReason )),
				this, SLOT(torrentSeedAutoStopped( bt::TorrentInterface*,bt::AutoStopReason )));
		connect(tc,SIGNAL(aboutToBeStarted( bt::TorrentInterface*,bool & )),
				this, SLOT(aboutToBeStarted( bt::TorrentInterface*,bool & )));
		connect(tc,SIGNAL(corruptedDataFound( bt::TorrentInterface* )),
				this, SLOT(emitCorruptedData( bt::TorrentInterface* )));	
		connect(tc, SIGNAL(needDataCheck(bt::TorrentInterface*)),
				this, SLOT(autoCheckData(bt::TorrentInterface*)));
		connect(tc,SIGNAL(statusChanged(bt::TorrentInterface*)),
				this,SLOT(onStatusChanged(bt::TorrentInterface*)));
	}

	float Core::getGlobalMaxShareRatio() const
	{
		return Settings::maxRatio();
	}

	void Core::enqueueTorrentOverMaxRatio(bt::TorrentInterface* tc)
	{
		emit queuingNotPossible(tc);
	}

	void Core::autoCheckData(bt::TorrentInterface* tc)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Doing an automatic data check on " 
					<< tc->getStats().torrent_name << endl;
		
		gui->dataScan(tc,false,true,QString::null);
	}

	void Core::doDataCheck(bt::TorrentInterface* tc)
	{
		bool dummy = false;
		if (tc->isCheckingData(dummy))
			return;
		
		gui->dataScan(tc,false,false,i18n("Checking Data Integrity"));
	}

	void Core::onLowDiskSpace(bt::TorrentInterface * tc, bool stopped)
	{
		emit lowDiskSpace(tc, stopped);
	}
	
	void Core::updateGuiPlugins()
	{
		pman->updateGuiPlugins();
	}
	
	void Core::checkForKDE3Torrents()
	{
		Settings::setOldTorrentsImported(true);
		Settings::self()->writeConfig();
#ifndef Q_WS_WIN // this is only necessary on linux and other unix variants which support KDE3	
		TorrentMigratorDlg mig(gui);
		Uint32 num = mig.findTorrentsToBeMigrated();
		if (num > 0)
		{
			if (KMessageBox::questionYesNo(gui,i18n("KTorrent has found %1 torrents from the KDE3 version of KTorrent, do you want to import them ?",num)) == KMessageBox::Yes)
			{
				mig.migrateFoundTorrents(qman);
				foreach (const QString & s,mig.getSuccessFullImports())
					loadExistingTorrent(s);
			}
		}
#endif 
	}

	void Core::importKDE3Torrents()
	{
		TorrentMigratorDlg mig(gui);
		Uint32 num = mig.findTorrentsToBeMigrated();
		if (num > 0)
		{
			mig.migrateFoundTorrents(qman);
			foreach (const QString & s,mig.getSuccessFullImports())
				loadExistingTorrent(s);
		}
		else
		{
			KMessageBox::information(gui,i18n("No torrents from the KDE3 version were found !"));
		}
	}
	
	DBus* Core::getExternalInterface()
	{
		return gui->getDBusInterface();
	}
	
	void Core::onStatusChanged(bt::TorrentInterface* tc)
	{
		Q_UNUSED(tc);
		gui->updateActions();
	}
}

#include "core.moc"

