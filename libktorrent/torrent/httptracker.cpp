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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <config.h>
 
#include <kurl.h>
#include <klocale.h>
#include <qhostaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/waitjob.h>
#include <interfaces/exitoperation.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include "bnode.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"
#include "peermanager.h"
#include "server.h"
#include "globals.h"
#include "settings.h"


using namespace kt;

namespace bt
{

	HTTPTracker::HTTPTracker(const KURL & url,kt::TorrentInterface* tor,const PeerID & id,int tier)
		: Tracker(url,tor,id,tier)
	{
		active_job = 0;
		
		interval = 5 * 60; // default interval 5 minutes
		failures = 0;
		seeders = leechers = 0;
	}


	HTTPTracker::~HTTPTracker()
	{
	}
	
	void HTTPTracker::start()
	{
		event = "started";
		doRequest();
	}
	
	void HTTPTracker::stop(WaitJob* wjob)
	{
		if (!started)
			return;
		
		event = "stopped";
		doRequest(wjob);
		started = false;
	}
	
	void HTTPTracker::completed()
	{
		event = "completed";
		doRequest();
		event = QString::null;
	}
	
	void HTTPTracker::manualUpdate()
	{
		if (!started)
			event = "started";
		doRequest();
	}
	
	void HTTPTracker::scrape()
	{
		if (!url.isValid())
		{
			Out(SYS_TRK|LOG_NOTICE) << "Invalid tracker url, canceling scrape" << endl;
			return;
		}
		
		if (!url.fileName(false).startsWith("announce"))
		{
			Out(SYS_TRK|LOG_NOTICE) << "Tracker " << url << " does not support scraping" << endl;
			return;
		}
		
		KURL scrape_url = url;
		scrape_url.setFileName(url.fileName(false).replace("announce","scrape"));
		
		QString epq = scrape_url.encodedPathAndQuery();
		const SHA1Hash & info_hash = tor->getInfoHash();
		if (scrape_url.queryItems().count() > 0)
			epq += "&info_hash=" + info_hash.toURLString();
		else
			epq += "?info_hash=" + info_hash.toURLString();
		scrape_url.setEncodedPathAndQuery(epq);
	
		Out(SYS_TRK|LOG_NOTICE) << "Doing scrape request to url : " << scrape_url.prettyURL() << endl;
		KIO::MetaData md;
		setupMetaData(md);
		
		KIO::StoredTransferJob* j = KIO::storedGet(scrape_url,false,false);
		// set the meta data
		j->setMetaData(md);
		KIO::Scheduler::scheduleJob(j);
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onScrapeResult( KIO::Job* )));
	}
	
	void HTTPTracker::onScrapeResult(KIO::Job* j)
	{
		if (j->error())
		{
			Out(SYS_TRK|LOG_IMPORTANT) << "Scrape failed : " << j->errorString() << endl;
			return;
		}
		
		KIO::StoredTransferJob* st = (KIO::StoredTransferJob*)j;
		BDecoder dec(st->data(),false,0);
		BNode* n = 0;
		
		try
		{
			n = dec.decode();
		}
		catch (bt::Error & err)
		{
			Out(SYS_TRK|LOG_IMPORTANT) << "Invalid scrape data " << err.toString() << endl;
			return;
		}
			
		if (n && n->getType() == BNode::DICT)
		{
			BDictNode* d = (BDictNode*)n;
			d = d->getDict("files");
			if (d)
			{
				d = d->getDict(tor->getInfoHash().toByteArray());
				if (d)
				{
					BValueNode* vn = d->getValue("complete");
					if (vn && vn->data().getType() == Value::INT)
					{
						seeders = vn->data().toInt();
					} 
						
					
					vn = d->getValue("incomplete");
					if (vn && vn->data().getType() == Value::INT)
					{
						leechers = vn->data().toInt();
					}
					
					Out(SYS_TRK|LOG_DEBUG) << "Scrape : leechers = " << leechers 
							<< ", seeders = " << seeders << endl;
				}
			}
		}
		
		delete n;
	}
	
	void HTTPTracker::doRequest(WaitJob* wjob)
	{	
		const TorrentStats & s = tor->getStats();
		
		KURL u = url;
		if (!url.isValid())
		{
			requestPending();
			QTimer::singleShot(500,this,SLOT(emitInvalidURLFailure()));
			return;
		}

		Uint16 port = Globals::instance().getServer().getPortInUse();;
		
		u.addQueryItem("peer_id",peer_id.toString());
		u.addQueryItem("port",QString::number(port));
		u.addQueryItem("uploaded",QString::number(s.trk_bytes_uploaded));
		u.addQueryItem("downloaded",QString::number(s.trk_bytes_downloaded));
		
		if (event == "completed")
			u.addQueryItem("left","0"); // need to send 0 when we are completed
		else
			u.addQueryItem("left",QString::number(s.bytes_left));
		
		u.addQueryItem("compact","1");
		if (event != "stopped")
			u.addQueryItem("numwant","100");
		else
			u.addQueryItem("numwant","0");
		
		u.addQueryItem("key",QString::number(key));
		QString cip = Tracker::getCustomIP();
		if (!cip.isNull())
			u.addQueryItem("ip",cip);

		if (event != QString::null)
			u.addQueryItem("event",event);
		QString epq = u.encodedPathAndQuery();
		const SHA1Hash & info_hash = tor->getInfoHash();
		epq += "&info_hash=" + info_hash.toURLString();


		u.setEncodedPathAndQuery(epq);
		
		if (active_job)
		{
			announce_queue.append(u);
			Out(SYS_TRK|LOG_NOTICE) << "Announce ongoing, queueing announce" << endl;
		}
		else
		{
			doAnnounce(u);
			// if there is a wait job, add this job to the waitjob 
			if (wjob)
				wjob->addExitOperation(new kt::ExitJobOperation(active_job));
		}
	}

	bool HTTPTracker::updateData(const QByteArray & data)
	{
//#define DEBUG_PRINT_RESPONSE
#ifdef DEBUG_PRINT_RESPONSE
		Out() << "Data : " << endl;
		Out() << QString(data) << endl;
#endif
		// search for dictionary, there might be random garbage infront of the data
		Uint32 i = 0;
		while (i < data.size())
		{
			if (data[i] == 'd')
				break;
			i++;
		}
		
		if (i == data.size())
		{
			failures++;
			requestFailed(i18n("Invalid response from tracker"));
			return false;
		}
		
		BDecoder dec(data,false,i);
		BNode* n = 0;
		try
		{
			n = dec.decode();
		}
		catch (...)
		{
			failures++;
			requestFailed(i18n("Invalid data from tracker"));
			return false;
		}
			
		if (!n || n->getType() != BNode::DICT)
		{
			failures++;
			requestFailed(i18n("Invalid response from tracker"));
			return false;
		}
			
		BDictNode* dict = (BDictNode*)n;
		if (dict->getData("failure reason"))
		{
			BValueNode* vn = dict->getValue("failure reason");
			QString msg = vn->data().toString();
			delete n;
			failures++;
			requestFailed(msg);
			return false;
		}
			
		BValueNode* vn = dict->getValue("interval");
			
		// if no interval is specified, use 5 minutes
		if (vn)
			interval = vn->data().toInt();
		else
			interval = 5 * 60;
			
		vn = dict->getValue("incomplete");
		if (vn)
			leechers = vn->data().toInt();

		vn = dict->getValue("complete");
		if (vn)
			seeders = vn->data().toInt();
	
		BListNode* ln = dict->getList("peers");
		if (!ln)
		{
			// no list, it might however be a compact response
			vn = dict->getValue("peers");
			if (!vn)
			{
				delete n;
				failures++;
				requestFailed(i18n("Invalid response from tracker"));
				return false;
			}

			QByteArray arr = vn->data().toByteArray();
			for (Uint32 i = 0;i < arr.size();i+=6)
			{
				Uint8 buf[6];
				for (int j = 0;j < 6;j++)
					buf[j] = arr[i + j];

				addPeer(QHostAddress(ReadUint32(buf,0)).toString(),ReadUint16(buf,4));
			}
		}
		else
		{
			for (Uint32 i = 0;i < ln->getNumChildren();i++)
			{
				BDictNode* dict = dynamic_cast<BDictNode*>(ln->getChild(i));

				if (!dict)
					continue;
				
				BValueNode* ip_node = dict->getValue("ip");
				BValueNode* port_node = dict->getValue("port");

				if (!ip_node || !port_node)
					continue;
				
				addPeer(ip_node->data().toString(),port_node->data().toInt());
			}
		}
		
		delete n;
		return true;
	}

	
	void HTTPTracker::onAnnounceResult(KIO::Job* j)
	{
		if (j->error())
		{
			KIO::StoredTransferJob* st = (KIO::StoredTransferJob*)j;
			KURL u = st->url();
			active_job = 0;
			
			Out(SYS_TRK|LOG_IMPORTANT) << "Error : " << st->errorString() << endl;
			if (u.queryItem("event") != "stopped")
			{
				failures++;
				requestFailed(j->errorString());
			}
			else
			{
				stopDone();
			}
		}
		else
		{
			KIO::StoredTransferJob* st = (KIO::StoredTransferJob*)j;
			KURL u = st->url();
			active_job = 0;
			
			if (u.queryItem("event") != "stopped")
			{
				try
				{
					if (updateData(st->data()))
					{
						failures = 0;
						peersReady(this);
						requestOK();
						if (u.queryItem("event") == "started")
							started = true;
					}
				}
				catch (bt::Error & err)
				{
					failures++;
					requestFailed(i18n("Invalid response from tracker"));
				}
				event = QString::null;
			}
			else
			{
				failures = 0;
				stopDone();
			}
		}
		doAnnounceQueue();
	}

	void HTTPTracker::emitInvalidURLFailure()
	{
		failures++;
		requestFailed(i18n("Invalid tracker URL"));
	}
	
	void HTTPTracker::setupMetaData(KIO::MetaData & md)
	{
		md["UserAgent"] = "ktorrent/" VERSION;
		md["SendLanguageSettings"] = "false";
		md["Cookies"] = "none";
	//	md["accept"] = "text/plain";
		md["accept"] = "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2";
		if (Settings::doNotUseKDEProxy())
		{
			// set the proxy if the doNotUseKDEProxy ix enabled (URL must be valid to)
			KURL url = KURL::fromPathOrURL(Settings::httpTrackerProxy());
			if (url.isValid())
				md["UseProxy"] = url.pathOrURL();
			else
				md["UseProxy"] = QString::null;
		}
	}
	
	void HTTPTracker::doAnnounceQueue()
	{
		if (announce_queue.empty())
			return;
		
		KURL u = announce_queue.front();
		announce_queue.pop_front();
		doAnnounce(u);
	}
	
	void HTTPTracker::doAnnounce(const KURL & u)
	{
		Out(SYS_TRK|LOG_NOTICE) << "Doing tracker request to url : " << u.prettyURL() << endl;
		KIO::MetaData md;
		setupMetaData(md);
		KIO::StoredTransferJob* j = KIO::storedGet(u,false,false);
		// set the meta data
		j->setMetaData(md);
		KIO::Scheduler::scheduleJob(j);
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onAnnounceResult( KIO::Job* )));
		
		active_job = j;
		requestPending();
	}
}
#include "httptracker.moc"
