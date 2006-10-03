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
#include <qdir.h>
#include <qfile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <qtextstream.h>
#include <util/log.h>
#include <util/error.h>
#include <util/bitset.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <interfaces/functions.h>
#include <interfaces/trackerslist.h>
#include <datachecker/singledatachecker.h>
#include <datachecker/multidatachecker.h>
#include <datachecker/datacheckerlistener.h>
#include <migrate/ccmigrate.h>
#include <migrate/cachemigrate.h>
#include <kademlia/dhtbase.h>
#include "downloader.h"
#include "uploader.h"
#include "peersourcemanager.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "peermanager.h"

#include "torrentfile.h"
#include "torrentcontrol.h"

#include "peer.h"
#include "choker.h"

#include "globals.h"
#include "server.h"
#include "packetwriter.h"
#include "httptracker.h"
#include "udptracker.h"
#include "downloadcap.h"
#include "uploadcap.h"
#include "queuemanager.h"
#include "statsfile.h"
#include "announcelist.h"
#include "preallocationthread.h"
#include "timeestimator.h"

using namespace kt;

namespace bt
{



	TorrentControl::TorrentControl()
	: tor(0),psman(0),cman(0),pman(0),down(0),up(0),choke(0),tmon(0),prealloc(false), last_announce(0)
	{
		stats.imported_bytes = 0;
		stats.trk_bytes_downloaded = 0;
		stats.trk_bytes_uploaded = 0;
		stats.running = false;
		stats.started = false;
		stats.stopped_by_error = false;
		stats.session_bytes_downloaded = 0;
		stats.session_bytes_uploaded = 0;
		session_bytes_uploaded = 0;
		old_datadir = QString::null;
		stats.status = NOT_STARTED;
		stats.autostart = true;
		stats.user_controlled = false;
		stats.priv_torrent = false;
		stats.seeders_connected_to = stats.seeders_total = 0;
		stats.leechers_connected_to = stats.leechers_total = 0;
		running_time_dl = running_time_ul = 0;
		prev_bytes_dl = 0;
		prev_bytes_ul = 0;
		trk_prev_bytes_dl = trk_prev_bytes_ul = 0;
		io_error = false;
		priority = 0;
		maxShareRatio = 0.00f;
		custom_output_name = false;
		updateStats();
		prealoc_thread = 0;
		
		m_eta = new TimeEstimator(this);
	}




	TorrentControl::~TorrentControl()
	{
		if (stats.running)
			stop(false);
		
		if (tmon)
			tmon->destroyed();
		delete choke;
		delete down;
		delete up;
		delete cman;
		delete pman;
		delete psman;
		delete tor;
		delete m_eta;
	}

	void TorrentControl::update()
	{
		// do not update during critical operation mode or when we are data checking
		if (Globals::instance().inCriticalOperationMode() || stats.status == kt::CHECKING_DATA)
			return;
		
		if (io_error)
		{
			stop(false);
			emit stoppedByError(this, error_msg);
			return;
		}
		
		if (prealoc_thread)
		{
			if (prealoc_thread->isDone())
			{
				// thread done
				if (prealoc_thread->errorHappened())
				{
					// upon error just call onIOError and return
					onIOError(prealoc_thread->errorMessage());
					delete prealoc_thread;
					prealoc_thread = 0;
					prealloc = true; // still need to do preallocation
					return;
				}
				else
				{
					// continue the startup of the torrent
					delete prealoc_thread;
					prealoc_thread = 0;
					prealloc = false;
					stats.status = kt::NOT_STARTED;
					saveStats();
					continueStart();
				}
			}
			else
				return; // preallocation still going on, so just return
		}
		
		try
		{
			// first update peermanager
			pman->update();
			bool comp = stats.completed;

			// then the downloader and uploader
			up->update(choke->getOptimisticlyUnchokedPeerID());
			//if (!completed)
			down->update();

			stats.completed = cman->completed();
			if (stats.completed && !comp)
			{
				// download has just been completed
				// only sent completed to tracker when we have all chunks (so no excluded chunks)
				if (cman->haveAllChunks())
					psman->completed();
				pman->killSeeders();
				QDateTime now = QDateTime::currentDateTime();
				running_time_dl += time_started_dl.secsTo(now);
				updateStatusMsg();
				updateStats();
				finished(this);
			}
			else if (!stats.completed && comp)
			{
				// restart download if necesarry
				// when user selects that files which were previously excluded,
				// should now be downloaded
				if (!psman->isStarted())
					psman->start();
				else
					psman->manualUpdate();
				last_announce = bt::GetCurrentTime();
				time_started_dl = QDateTime::currentDateTime();
			}
			updateStatusMsg();

			// get rid of dead Peers
			Uint32 num_cleared = pman->clearDeadPeers();
			
			// we may need to update the choker
			if (choker_update_timer.getElapsedSinceUpdate() >= 10000 || num_cleared > 0)
			{
				// also get rid of seeders when download is finished
				// no need to keep them around, but also no need to do this
				// every update, so once every 10 seconds is fine
				if (stats.completed)
					pman->killSeeders();
				
				doChoking();
				choker_update_timer.update();
				// a good opportunity to make sure we are not keeping to much in memory
				cman->checkMemoryUsage();
			}

			// to satisfy people obsessed with their share ratio
			if (stats_save_timer.getElapsedSinceUpdate() >= 5*60*1000)
			{
				saveStats();
				stats_save_timer.update();
			}

			// Update DownloadCap
			updateStats();
			if (stats.download_rate > 0)
				stalled_timer.update();
			
			// do a manual update if we are stalled for more then 2 minutes
			// we do not do this for private torrents
			if (stalled_timer.getElapsedSinceUpdate() > 120000 && stats.bytes_left > 0 &&
				!stats.priv_torrent)
			{
				Out() << "Stalled for too long, time to get some fresh blood" << endl;
				psman->manualUpdate();
				stalled_timer.update();
			}
			
			if(overMaxRatio())
			{
				stop(true);
				emit seedingAutoStopped(this);
			}
			
		}
		catch (Error & e)
		{
			onIOError(e.toString());
		}
	}

	void TorrentControl::onIOError(const QString & msg)
	{
		Out() << "Error : " << msg << endl;
		stats.stopped_by_error = true;
		stats.status = ERROR;
		error_msg = msg;
		short_error_msg = msg;
		io_error = true;
	}

	void TorrentControl::start()
	{	
		// do not start running torrents
		if (stats.running || stats.status == kt::ALLOCATING_DISKSPACE)
			return;

		stats.stopped_by_error = false;
		io_error = false;
		try
		{
			bool ret = true;
			aboutToBeStarted(this,ret);
			if (!ret)
				return;
		}
		catch (Error & err)
		{
			// something went wrong when files were recreated, set error and rethrow
			onIOError(err.toString());
			return;
		}
		
		try
		{
			cman->start();
		}
		catch (Error & e)
		{
			onIOError(e.toString());
			throw;
		}
		
		time_started_ul = time_started_dl = QDateTime::currentDateTime();
		resetTrackerStats();
		
		if (prealloc)
		{
			Out(SYS_GEN|LOG_NOTICE) << "Pre-allocating diskspace" << endl;
			prealoc_thread = new PreallocationThread(cman);
			stats.running = true;
			stats.status = kt::ALLOCATING_DISKSPACE;
			prealoc_thread->start();
			return;
		}
		
		continueStart();
	}
	
	void TorrentControl::continueStart()
	{
		// continues start after the prealoc_thread has finished preallocation
		pman->start();
		try
		{
			down->loadDownloads(datadir + "current_chunks");
		}
		catch (Error & e)
		{
			// print out warning in case of failure
			// we can still continue the download
			Out(SYS_GEN|LOG_NOTICE) << "Warning : " << e.toString() << endl;
		}
		
		loadStats();
		stats.running = true;
		stats.started = true;
		stats.autostart = true;
		choker_update_timer.update();
		stats_save_timer.update();
		
		
		stalled_timer.update();
		psman->start();
		last_announce = bt::GetCurrentTime();
		stalled_timer.update();
	}
		

	void TorrentControl::stop(bool user)
	{
		QDateTime now = QDateTime::currentDateTime();
		if(!stats.completed)
			running_time_dl += time_started_dl.secsTo(now);
		running_time_ul += time_started_ul.secsTo(now);
		time_started_ul = time_started_dl = now;
		
		// stop preallocation thread if necesarry
		if (prealoc_thread)
		{
			prealoc_thread->stop();
			prealoc_thread->wait();
			
			if (prealoc_thread->errorHappened() || prealoc_thread->isNotFinished())
			{
				delete prealoc_thread;
				prealoc_thread = 0;
				prealloc = true;
				saveStats(); // save stats, so that we will start preallocating the next time
			}
			else
			{
				delete prealoc_thread;
				prealoc_thread = 0;
				prealloc = false;
			}
		}
	
		if (stats.running)
		{
			psman->stop();

			if (tmon)
				tmon->stopped();

			try
			{
				down->saveDownloads(datadir + "current_chunks");
			}
			catch (Error & e)
			{
				// print out warning in case of failure
				// it doesn't corrupt the data, so just a couple of lost chunks
				Out() << "Warning : " << e.toString() << endl;
			}
			
			down->clearDownloads();
			if (user)
			{
				//make this torrent user controlled
				setPriority(0);
				stats.autostart = false;
			}
		}
		pman->stop();
		pman->closeAllConnections();
		pman->clearDeadPeers();
		cman->stop();
		
		stats.running = false;
		saveStats();
		updateStatusMsg();
		updateStats();
		stats.trk_bytes_downloaded = 0;
		stats.trk_bytes_uploaded = 0;
	}

	void TorrentControl::setMonitor(kt::MonitorInterface* tmo)
	{
		tmon = tmo;
		down->setMonitor(tmon);
		if (tmon)
		{
			for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
				tmon->peerAdded(pman->getPeer(i));
		}
	}
	
	
	void TorrentControl::init(QueueManager* qman,
							  const QString & torrent,
							  const QString & tmpdir,
							  const QString & ddir,
							  const QString & default_save_dir)
	{
		// first load the torrent file
		tor = new Torrent();
		try
		{
			tor->load(torrent,false);
		}
		catch (...)
		{
			delete tor;
			tor = 0;
			throw Error(i18n("An error occurred while loading the torrent."
					" The torrent is probably corrupt or is not a torrent file."));
		}
		
		initInternal(qman,tmpdir,ddir,default_save_dir,torrent.startsWith(tmpdir));
		
		// copy torrent in tor dir
		QString tor_copy = datadir + "torrent";
		if (tor_copy != torrent)
		{
			bt::CopyFile(torrent,tor_copy);
		}
	}
	
	
	void TorrentControl::init(QueueManager* qman, const QByteArray & data,const QString & tmpdir,
							  const QString & ddir,const QString & default_save_dir)
	{
		// first load the torrent file
		tor = new Torrent();
		try
		{
			tor->load(data,false);
		}
		catch (...)
		{
			delete tor;
			tor = 0;
			throw Error(i18n("An error occurred while loading the torrent."
					" The torrent is probably corrupt or is not a torrent file."));
		}
		
		initInternal(qman,tmpdir,ddir,default_save_dir,true);
		// copy data into torrent file
		QString tor_copy = datadir + "torrent";
		QFile fptr(tor_copy);
		if (!fptr.open(IO_WriteOnly))
			throw Error(i18n("Unable to create %1 : %2")
					.arg(tor_copy).arg(fptr.errorString()));
	
		fptr.writeBlock(data.data(),data.size());
	}
	
	void TorrentControl::checkExisting(QueueManager* qman)
	{
		// check if we haven't already loaded the torrent
		// only do this when qman isn't 0
		if (qman && qman->allreadyLoaded(tor->getInfoHash()))
		{
			if (!stats.priv_torrent)
			{
				qman->mergeAnnounceList(tor->getInfoHash(),tor->getTrackerList());

				throw Error(i18n("You are already downloading this torrent %1, the list of trackers of both torrents has been merged.").arg(tor->getNameSuggestion()));
			}
			else
			{
				throw Error(i18n("You are already downloading the torrent %1")
						.arg(tor->getNameSuggestion()));
			}
		}
	}
	
	void TorrentControl::setupDirs(const QString & tmpdir,const QString & ddir)
	{
		datadir = tmpdir;
		
		if (!datadir.endsWith(DirSeparator()))
			datadir += DirSeparator();

		outputdir = ddir.stripWhiteSpace();
		if (outputdir.length() > 0 && !outputdir.endsWith(DirSeparator()))
			outputdir += DirSeparator();
		
		if (!bt::Exists(datadir))
		{
			bt::MakeDir(datadir);
		}
	}
	
	void TorrentControl::setupStats()
	{
		stats.completed = false;
		stats.running = false;
		stats.torrent_name = tor->getNameSuggestion();
		stats.multi_file_torrent = tor->isMultiFile();
		stats.total_bytes = tor->getFileLength();
		stats.priv_torrent = tor->isPrivate();
		
		// check the stats file for the custom_output_name variable
		StatsFile st(datadir + "stats");
		if (st.hasKey("CUSTOM_OUTPUT_NAME") && st.readULong("CUSTOM_OUTPUT_NAME") == 1)
		{
			custom_output_name = true;
		}
		
		// load outputdir if outputdir is null
		if (outputdir.isNull() || outputdir.length() == 0)
			loadOutputDir();
	}
	
	void TorrentControl::setupData(const QString & ddir)
	{
		// create PeerManager and Tracker
		pman = new PeerManager(*tor);
		//Out() << "Tracker url " << url << " " << url.protocol() << " " << url.prettyURL() << endl;
		psman = new PeerSourceManager(this,pman);
		connect(psman,SIGNAL(statusChanged( const QString& )),
				this,SLOT(trackerStatusChanged( const QString& )));


		// Create chunkmanager, load the index file if it exists
		// else create all the necesarry files
		cman = new ChunkManager(*tor,datadir,outputdir,custom_output_name);
		// outputdir is null, see if the cache has figured out what it is
		if (outputdir.length() == 0)
			outputdir = cman->getDataDir();
		
		// store the outputdir into the output_path variable, so others can access it	
		
		connect(cman,SIGNAL(updateStats()),this,SLOT(updateStats()));
		if (bt::Exists(datadir + "index"))
			cman->loadIndexFile();

	
		// as a sanity check make sure all files are created properly the first time we load this torrent
		if (!stats.completed && !ddir.isNull())
			cman->createFiles();

		stats.completed = cman->completed();

		// create downloader,uploader and choker
		down = new Downloader(*tor,*pman,*cman);
		connect(down,SIGNAL(ioError(const QString& )),
				this,SLOT(onIOError(const QString& )));
		up = new Uploader(*cman,*pman);
		choke = new Choker(*pman,*cman);


		connect(pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerRemoved(Peer* )));
		connect(cman,SIGNAL(excluded(Uint32, Uint32 )),down,SLOT(onExcluded(Uint32, Uint32 )));
		connect(cman,SIGNAL(included( Uint32, Uint32 )),down,SLOT(onIncluded( Uint32, Uint32 )));
		connect(cman,SIGNAL(corrupted( Uint32 )),this,SLOT(corrupted( Uint32 )));
	}
	
	void TorrentControl::initInternal(QueueManager* qman,
									  const QString & tmpdir,
									  const QString & ddir,
									  const QString & default_save_dir,
									  bool first_time)
	{
		checkExisting(qman);
		setupDirs(tmpdir,ddir);
		setupStats();

		if (!first_time)
		{
			// if we do not need to copy the torrent, it is an existing download and we need to see
			// if it is not an old download
			try
			{
				migrateTorrent(default_save_dir);
			}
			catch (Error & err)
			{
				
				throw Error(
						i18n("Cannot migrate %1 : %2")
						.arg(tor->getNameSuggestion()).arg(err.toString()));
			}
		}
		setupData(ddir);

		updateStatusMsg();

		// to get rid of phantom bytes we need to take into account
		// the data from downloads already in progress
		try
		{
			Uint64 db = down->bytesDownloaded();
			Uint64 cb = down->getDownloadedBytesOfCurrentChunksFile(datadir + "current_chunks");
			prev_bytes_dl = db + cb;
				
		//	Out() << "Downloaded : " << kt::BytesToString(db) << endl;
		//	Out() << "current_chunks : " << kt::BytesToString(cb) << endl;
		}
		catch (Error & e)
		{
			// print out warning in case of failure
			Out() << "Warning : " << e.toString() << endl;
			prev_bytes_dl = down->bytesDownloaded();
		}
		
		loadStats();
		updateStats();
		saveStats();
		stats.output_path = cman->getOutputPath();
		Out() << "OutputPath = " << stats.output_path << endl;
	}
	


	bool TorrentControl::announceAllowed()
	{
		if(last_announce == 0)
			return true;
		
		if (psman && psman->getNumFailures() == 0)
			return bt::GetCurrentTime() - last_announce >= 60 * 1000;
		else
			return true;
	}
	
	void TorrentControl::updateTracker()
	{
		if (stats.running && announceAllowed())
		{
			psman->manualUpdate();
			last_announce = bt::GetCurrentTime();
		}
	}

	void TorrentControl::onNewPeer(Peer* p)
	{
		connect(p,SIGNAL(gotPortPacket( const QString&, Uint16 )),
				this,SLOT(onPortPacket( const QString&, Uint16 )));
		
		if (p->getStats().fast_extensions)
		{
			const BitSet & bs = cman->getBitSet();
			if (bs.allOn())
				p->getPacketWriter().sendHaveAll();
			else if (bs.numOnBits() == 0)
				p->getPacketWriter().sendHaveNone();
			else
				p->getPacketWriter().sendBitSet(bs);
		}
		else
		{
			p->getPacketWriter().sendBitSet(cman->getBitSet());
		}
		
		if (!stats.completed)
			p->getPacketWriter().sendInterested();
		
		if (p->isDHTSupported() && !stats.priv_torrent)
			p->getPacketWriter().sendPort(Globals::instance().getDHT().getPort());
		
		if (tmon)
			tmon->peerAdded(p);
	}

	void TorrentControl::onPeerRemoved(Peer* p)
	{
		disconnect(p,SIGNAL(gotPortPacket( const QString&, Uint16 )),
				this,SLOT(onPortPacket( const QString&, Uint16 )));
		if (tmon)
			tmon->peerRemoved(p);
	}

	void TorrentControl::doChoking()
	{
		choke->update(stats.completed,stats);
	}

	bool TorrentControl::changeDataDir(const QString & new_dir)
	{
#warning "This code is broken at the moment, will fix later"
		return false;
		// new_dir doesn't contain the torX/ part
		// so first get that and append it to new_dir
		int dd = datadir.findRev(DirSeparator(),datadir.length() - 2,false);
		QString tor = datadir.mid(dd + 1,datadir.length() - 2 - dd);


		// make sure nd ends with a /
		QString nd = new_dir + tor;
		if (!nd.endsWith(DirSeparator()))
			nd += DirSeparator();

		Out() << datadir << " -> " << nd << endl;

		int ok_calls = 0;
		try
		{
			if (!bt::Exists(nd))
				bt::MakeDir(nd);

			// now try to move all the files :
			// first the torrent
			bt::Move(datadir + "torrent",nd);
			ok_calls++;
			// then the index
			bt::Move(datadir + "index",nd);
			ok_calls++;
			// then the cache
			bt::Move(datadir + "cache",nd);
			ok_calls++;

			// tell the chunkmanager that the datadir has changed
			cman->changeDataDir(nd);
		}
		catch (...)
		{
			// move the torrent back
			if (ok_calls >= 1)
				bt::Move(nd + "torrent",datadir,true);
			if (ok_calls >= 2)
				bt::Move(nd + "index",datadir,true);
			return false;
		}

		// we don't move the current_chunks file
		// it will be recreated anyway
		// now delete the old directory
		bt::Delete(datadir,true);

		old_datadir = datadir;
		datadir = nd;
		return true;
	}


	void TorrentControl::rollback()
	{
		if (old_datadir.isNull())
			return;

		// recreate it
		if (!bt::Exists(old_datadir))
			bt::MakeDir(old_datadir,true);

		// move back files
		bt::Move(datadir + "torrent",old_datadir,true);
		bt::Move(datadir + "cache",old_datadir,true);
		bt::Move(datadir + "index",old_datadir,true);
		cman->changeDataDir(old_datadir);

		// delete new
		bt::Delete(datadir,true);

		datadir = old_datadir;
		old_datadir = QString::null;
	}

	void TorrentControl::updateStatusMsg()
	{
		if (stats.stopped_by_error)
			stats.status = kt::ERROR;
		else if (!stats.started)
			stats.status = kt::NOT_STARTED;
		else if(!stats.running && !stats.user_controlled)
			stats.status = kt::QUEUED;
		else if (!stats.running && stats.completed)
			stats.status = kt::COMPLETE;
		else if (!stats.running)
			stats.status = kt::STOPPED;
		else if (stats.running && stats.completed)
			stats.status = kt::SEEDING;
		else if (stats.running)
			stats.status = down->downloadRate() > 0 ?
					kt::DOWNLOADING : kt::STALLED;
	}

	const BitSet & TorrentControl::downloadedChunksBitSet() const
	{
		if (cman)
			return cman->getBitSet();
		else
			return BitSet::null;
	}

	const BitSet & TorrentControl::availableChunksBitSet() const
	{
		if (!pman)
			return BitSet::null;
		else
			return pman->getAvailableChunksBitSet();
	}

	const BitSet & TorrentControl::excludedChunksBitSet() const
	{
		if (!cman)
			return BitSet::null;
		else
			return cman->getExcludedBitSet();
	}

	void TorrentControl::saveStats()
	{
		StatsFile st(datadir + "stats");

		st.write("OUTPUTDIR", cman->getDataDir());
		
		if (cman->getDataDir() != outputdir)
			outputdir = cman->getDataDir();
		
		st.write("UPLOADED", QString::number(up->bytesUploaded()));
		
		if (stats.running)
		{
			QDateTime now = QDateTime::currentDateTime();
			st.write("RUNNING_TIME_DL",QString("%1").arg(running_time_dl + time_started_dl.secsTo(now)));
			st.write("RUNNING_TIME_UL",QString("%1").arg(running_time_ul + time_started_ul.secsTo(now)));
		}
		else
		{
			st.write("RUNNING_TIME_DL", QString("%1").arg(running_time_dl));
			st.write("RUNNING_TIME_UL", QString("%1").arg(running_time_ul));
		}
		
		st.write("PRIORITY", QString("%1").arg(priority));
		st.write("AUTOSTART", QString("%1").arg(stats.autostart));
		st.write("IMPORTED", QString("%1").arg(stats.imported_bytes));
		st.write("CUSTOM_OUTPUT_NAME",custom_output_name ? "1" : "0");
		st.write("MAX_RATIO", QString("%1").arg(maxShareRatio,0,'f',2));
		st.write("RESTART_DISK_PREALLOCATION",prealloc ? "1" : "0");
		
		st.writeSync();
	}

	void TorrentControl::loadStats()
	{
		StatsFile st(datadir + "stats");
		
		Uint64 val = st.readUint64("UPLOADED");
		// stats.session_bytes_uploaded will be calculated based upon prev_bytes_ul
		// seeing that this will change here, we need to save it 
		session_bytes_uploaded = stats.session_bytes_uploaded; 
		prev_bytes_ul = val;
		up->setBytesUploaded(val);
		
		this->running_time_dl = st.readULong("RUNNING_TIME_DL");
		this->running_time_ul = st.readULong("RUNNING_TIME_UL");
		outputdir = st.readString("OUTPUTDIR").stripWhiteSpace();
		if (st.hasKey("CUSTOM_OUTPUT_NAME") && st.readULong("CUSTOM_OUTPUT_NAME") == 1)
		{
			custom_output_name = true;
		}
		
		setPriority(st.readInt("PRIORITY"));
		stats.user_controlled = priority == 0 ? true : false;
		stats.autostart = st.readBoolean("AUTOSTART");
		
		stats.imported_bytes = st.readUint64("IMPORTED");
		float rat = st.readFloat("MAX_RATIO");
		maxShareRatio = rat;
		if (st.hasKey("RESTART_DISK_PREALLOCATION"))
			prealloc = st.readString("RESTART_DISK_PREALLOCATION") == "1";
		
		return;
	}

	void TorrentControl::loadOutputDir()
	{
		StatsFile st(datadir + "stats");
		if (!st.hasKey("OUTPUTDIR"))
			return;
		
		outputdir = st.readString("OUTPUTDIR").stripWhiteSpace();
		if (st.hasKey("CUSTOM_OUTPUT_NAME") && st.readULong("CUSTOM_OUTPUT_NAME") == 1)
		{
			custom_output_name = true;
		}
	}

	bool TorrentControl::readyForPreview(int start_chunk, int end_chunk)
	{
		if ( !tor->isMultimedia() && !tor->isMultiFile()) return false;

		const BitSet & bs = downloadedChunksBitSet();
		for(int i = start_chunk; i<end_chunk; ++i)
		{
			if ( !bs.get(i) ) return false;
		}
		return true;
	}

	Uint32 TorrentControl::getTimeToNextTrackerUpdate() const
	{
		if (psman)
			return psman->getTimeToNextUpdate();
		else
			return 0;
	}

	void TorrentControl::updateStats()
	{
		stats.num_chunks_downloading = down ? down->numActiveDownloads() : 0;
		stats.num_peers = pman ? pman->getNumConnectedPeers() : 0;
		stats.upload_rate = up && stats.running ? up->uploadRate() : 0;
		stats.download_rate = down && stats.running ? down->downloadRate() : 0;
		stats.bytes_left = cman ? cman->bytesLeft() : 0;
		stats.bytes_left_to_download = cman ? cman->bytesLeft() - cman->bytesExcluded() : 0;
		stats.bytes_uploaded = up ? up->bytesUploaded() : 0;
		stats.bytes_downloaded = down ? down->bytesDownloaded() : 0;
		stats.total_chunks = cman ? cman->getNumChunks() : 0;
		stats.num_chunks_downloaded = cman ? cman->chunksDownloaded() : 0;
		stats.num_chunks_excluded = cman ? cman->chunksExcluded() : 0;
		stats.chunk_size = tor ? tor->getChunkSize() : 0;
		stats.total_bytes_to_download = (tor && cman) ?	tor->getFileLength() - cman->bytesExcluded() : 0;
		
		
		
		if (stats.bytes_downloaded >= prev_bytes_dl)
			stats.session_bytes_downloaded = stats.bytes_downloaded - prev_bytes_dl;
		else
			stats.session_bytes_downloaded = 0;
					
		if (stats.bytes_uploaded >= prev_bytes_ul)
			stats.session_bytes_uploaded = (stats.bytes_uploaded - prev_bytes_ul) + session_bytes_uploaded;
		else
			stats.session_bytes_uploaded = session_bytes_uploaded;
		/*
			Safety check, it is possible that stats.bytes_downloaded gets subtracted in Downloader.
			Which can cause stats.bytes_downloaded to be smaller the trk_prev_bytes_dl.
			This can screw up your download ratio.
		*/
		if (stats.bytes_downloaded >= trk_prev_bytes_dl)
			stats.trk_bytes_downloaded = stats.bytes_downloaded - trk_prev_bytes_dl;
		else
			stats.trk_bytes_downloaded = 0;
		
		if (stats.bytes_uploaded >= trk_prev_bytes_ul)
			stats.trk_bytes_uploaded = stats.bytes_uploaded - trk_prev_bytes_ul;
		else
			stats.trk_bytes_uploaded = 0;
		
		getSeederInfo(stats.seeders_total,stats.seeders_connected_to);
		getLeecherInfo(stats.leechers_total,stats.leechers_connected_to);
	}

	void TorrentControl::getSeederInfo(Uint32 & total,Uint32 & connected_to) const
	{
		total = 0;
		connected_to = 0;
		if (!pman || !psman)
			return;

		for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
		{
			if (pman->getPeer(i)->isSeeder())
				connected_to++;
		}
		total = psman->getNumSeeders();
		if (total == 0)
			total = connected_to;
	}

	void TorrentControl::getLeecherInfo(Uint32 & total,Uint32 & connected_to) const
	{
		total = 0;
		connected_to = 0;
		if (!pman || !psman)
			return;

		for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
		{
			if (!pman->getPeer(i)->isSeeder())
				connected_to++;
		}
		total = psman->getNumLeechers();
		if (total == 0)
			total = connected_to;
	}

	Uint32 TorrentControl::getRunningTimeDL() const
	{
		if (!stats.running || stats.completed)
			return running_time_dl;
		else
			return running_time_dl + time_started_dl.secsTo(QDateTime::currentDateTime());
	}

	Uint32 TorrentControl::getRunningTimeUL() const
	{
		if (!stats.running)
			return running_time_ul;
		else
			return running_time_ul + time_started_ul.secsTo(QDateTime::currentDateTime());
	}

	Uint32 TorrentControl::getNumFiles() const
	{
		if (tor && tor->isMultiFile())
			return tor->getNumFiles();
		else
			return 0;
	}
	
	TorrentFileInterface & TorrentControl::getTorrentFile(Uint32 index)
	{
		if (tor)
			return tor->getFile(index);
		else
			return TorrentFile::null;
	}

	void TorrentControl::migrateTorrent(const QString & default_save_dir)
	{
		if (bt::Exists(datadir + "current_chunks") && bt::IsPreMMap(datadir + "current_chunks"))
		{
			// in case of error copy torX dir to migrate-failed-tor
			QString dd = datadir;
			int pos = dd.findRev("tor");
			if (pos != - 1)
			{
				dd = dd.replace(pos,3,"migrate-failed-tor");
				Out() << "Copying " << datadir << " to " << dd << endl;
				bt::CopyDir(datadir,dd,true);
			}
				
			bt::MigrateCurrentChunks(*tor,datadir + "current_chunks");
			if (outputdir.isNull() && bt::IsCacheMigrateNeeded(*tor,datadir + "cache"))
			{
				// if the output dir is NULL
				if (default_save_dir.isNull())
				{
					KMessageBox::information(0,
						i18n("The torrent %1 was started with a previous version of KTorrent."
							" To make sure this torrent still works with this version of KTorrent, "
							"we will migrate this torrent. You will be asked for a location to save "
							"the torrent to. If you press cancel, we will select your home directory.")
								.arg(tor->getNameSuggestion()));
					outputdir = KFileDialog::getExistingDirectory(QString::null, 0,i18n("Select Folder to Save To"));
					if (outputdir.isNull())
						outputdir = QDir::homeDirPath();
				}
				else
				{
					outputdir = default_save_dir;
				}
				
				if (!outputdir.endsWith(bt::DirSeparator()))
					outputdir += bt::DirSeparator();
				
				bt::MigrateCache(*tor,datadir + "cache",outputdir);
			}
			
			// delete backup
			if (pos != - 1)
				bt::Delete(dd);
		}
	}
	
	void TorrentControl::setPriority(int p)
	{
		priority = p;
		stats.user_controlled = p == 0 ? true : false;
		if(p)
			stats.status = kt::QUEUED;
		else
			updateStatusMsg();
		
		saveStats();
	}
	
	void TorrentControl::setMaxShareRatio(float ratio)
	{
		if(ratio == 1.00f)
		{
			if(maxShareRatio != ratio)
				maxShareRatio = ratio;
		}
		else
			maxShareRatio = ratio;
		
		saveStats();
		emit maxRatioChanged(this);
	}
	
	bool TorrentControl::overMaxRatio()
	{
		if(stats.completed && stats.bytes_uploaded != 0 && stats.bytes_downloaded != 0 && maxShareRatio > 0)
		{
			if(kt::ShareRatio(stats) >= maxShareRatio)
				return true;
		}
		
		return false;
	}

	
	QString TorrentControl::statusToString() const
	{
		switch (stats.status)
		{
			case kt::NOT_STARTED :
				return i18n("Not started");
			case kt::COMPLETE :
				return i18n("Completed");
			case kt::SEEDING :
				return i18n("Seeding");
			case kt::DOWNLOADING:
				return i18n("Downloading");
			case kt::STALLED:
				return i18n("Stalled");
			case kt::STOPPED:
				return i18n("Stopped");
			case kt::ERROR :
				return i18n("Error: ") + getShortErrorMessage(); 
			case kt::ALLOCATING_DISKSPACE:
				return i18n("Allocating diskspace");
			case kt::QUEUED:
				return i18n("Queued");
			case kt::CHECKING_DATA:
				return i18n("Checking data");
		}
		return QString::null;
	}

	TrackersList* TorrentControl::getTrackersList()
	{
		return psman;
	}
	
	const TrackersList* TorrentControl::getTrackersList() const 
	{
		return psman;
	}

	void TorrentControl::onPortPacket(const QString & ip,Uint16 port)
	{
		if (Globals::instance().getDHT().isRunning() && !stats.priv_torrent)
			Globals::instance().getDHT().portRecieved(ip,port);
	}
	
	void TorrentControl::doDataCheck(bt::DataCheckerListener* lst,bool auto_import)
	{
		if (stats.status == kt::ALLOCATING_DISKSPACE)
			return;
		
		DataChecker* dc = 0;
		stats.status = kt::CHECKING_DATA;
		if (stats.multi_file_torrent)
			dc = new MultiDataChecker();
		else
			dc = new SingleDataChecker();
	
		dc->setListener(lst);
		dc->check(stats.output_path,*tor,datadir + "dnd" + bt::DirSeparator());
		
		if (lst && !lst->isStopped())
		{
			down->dataChecked(dc->getDownloaded());
			// update chunk manager
			cman->dataChecked(dc->getDownloaded());
			if (auto_import)
			{
				down->recalcDownloaded();
				stats.imported_bytes = down->bytesDownloaded();
				if (cman->haveAllChunks())
					stats.completed = true;
			}
		}
		delete dc;
		// update the status
		updateStatusMsg();
		updateStats();
	}
	
	bool TorrentControl::hasExistingFiles() const
	{
		return cman->hasExistingFiles();
	}
	
	bool TorrentControl::hasMissingFiles(QStringList & sl)
	{
		return cman->hasMissingFiles(sl);
	}
	
	void TorrentControl::recreateMissingFiles()
	{
		try
		{
			cman->recreateMissingFiles();
			prealloc = true; // set prealloc to true so files will be truncated again
			down->dataChecked(cman->getBitSet()); // update chunk selector
		}
		catch (Error & err)
		{
			onIOError(err.toString());
			throw;
		}
	}
	
	void TorrentControl::dndMissingFiles()
	{
		try
		{
			cman->dndMissingFiles();
			prealloc = true; // set prealloc to true so files will be truncated again
			missingFilesMarkedDND(this);
			down->dataChecked(cman->getBitSet()); // update chunk selector
		}
		catch (Error & err)
		{
			onIOError(err.toString());
			throw;
		}
	}
	
	void TorrentControl::handleError(const QString & err)
	{
		onIOError(err);
	}
	
	Uint32 TorrentControl::getNumDHTNodes() const
	{
		return tor->getNumDHTNodes();
	}
	
	const kt::DHTNode & TorrentControl::getDHTNode(Uint32 i) const 
	{
		return tor->getDHTNode(i);
	}

	void TorrentControl::deleteDataFiles()
	{
		cman->deleteDataFiles();
	}
	
	const bt::SHA1Hash & TorrentControl::getInfoHash() const
	{
		return tor->getInfoHash();
	}
	
	void TorrentControl::resetTrackerStats()
	{
		trk_prev_bytes_dl = stats.bytes_downloaded,
		trk_prev_bytes_ul = stats.bytes_uploaded,
		stats.trk_bytes_downloaded = 0;
		stats.trk_bytes_uploaded = 0;
	}
	
	void TorrentControl::trackerStatusChanged(const QString & ns)
	{
		stats.trackerstatus = ns;
	}
	
	void TorrentControl::addPeerSource(kt::PeerSource* ps)
	{
		if (psman)
			psman->addPeerSource(ps);
	}
	
	void TorrentControl::removePeerSource(kt::PeerSource* ps)
	{
		if (psman)
			psman->removePeerSource(ps);
	}
	
	void TorrentControl::corrupted(Uint32 chunk)
	{
		// make sure we will redownload the chunk
		down->corrupted(chunk);
		if (stats.completed)
			stats.completed = false;
		
		// emit signal to show a systray message
		corruptedDataFound(this);
	}
	
	Uint32 TorrentControl::getETA()
	{
		return m_eta->estimate();
	}
}

#include "torrentcontrol.moc"
