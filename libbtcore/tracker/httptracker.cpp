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
#include "httptracker.h"
#include <config-btcore.h>
 
#include <kurl.h>
#include <klocale.h>
#include <k3socketaddress.h>
#include <qhostaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/waitjob.h>
#include <interfaces/exitoperation.h>
#include <interfaces/torrentinterface.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <peer/peermanager.h>
#include <torrent/server.h>
#include <torrent/globals.h>
#include "btversion.h"



namespace bt
{
	bool HTTPTracker::proxy_on = false;
	QString HTTPTracker::proxy = QString();
	Uint16 HTTPTracker::proxy_port = 8080;

	HTTPTracker::HTTPTracker(const KUrl & url,TorrentInterface* tor,const PeerID & id,int tier)
		: Tracker(url,tor,id,tier)
	{
		active_job = 0;
		
		interval = 5 * 60; // default interval 5 minutes
		failures = 0;
		seeders = leechers = 0;
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
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
		event = QString();
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
		
		if (!url.fileName().startsWith("announce"))
		{
			Out(SYS_TRK|LOG_NOTICE) << "Tracker " << url << " does not support scraping" << endl;
			return;
		}
		
		KUrl scrape_url = url;
		scrape_url.setFileName(url.fileName().replace("announce","scrape"));
		
		QString epq = scrape_url.encodedPathAndQuery();
		const SHA1Hash & info_hash = tor->getInfoHash();
		if (scrape_url.queryItems().count() > 0)
			epq += "&info_hash=" + info_hash.toURLString();
		else
			epq += "?info_hash=" + info_hash.toURLString();
		scrape_url.setEncodedPathAndQuery(epq);
	
		Out(SYS_TRK|LOG_NOTICE) << "Doing scrape request to url : " << scrape_url.prettyUrl() << endl;
		KIO::MetaData md;
		setupMetaData(md);
		
		KIO::StoredTransferJob* j = KIO::storedGet(scrape_url, KIO::NoReload, KIO::HideProgressInfo);
		// set the meta data
		j->setMetaData(md);
		KIO::Scheduler::scheduleJob(j);
		
		connect(j,SIGNAL(result(KJob* )),this,SLOT(onScrapeResult( KJob* )));
	}
	
	void HTTPTracker::onScrapeResult(KJob* j)
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
			d = d->getDict(QString("files"));
			if (d)
			{
				d = d->getDict(tor->getInfoHash().toByteArray());
				if (d)
				{
					try
					{
						seeders = d->getInt("complete");
						leechers = d->getInt("incomplete");
						total_downloaded = d->getInt("downloaded");
						Out(SYS_TRK|LOG_DEBUG) << "Scrape : leechers = " << leechers 
							<< ", seeders = " << seeders << ", downloaded = " << total_downloaded << endl;
					}
					catch (...)
					{}
					scrapeDone();
				}
			}
		}
		
		delete n;
	}
	
	void HTTPTracker::doRequest(WaitJob* wjob)
	{	
		const TorrentStats & s = tor->getStats();
		
		KUrl u = url;
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

		if (event != QString())
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
 				wjob->addExitOperation(new ExitJobOperation(active_job));
 		}
	}

	bool HTTPTracker::updateData(const QByteArray & data)
	{
//#define DEBUG_PRINT_RESPONSE
#ifdef DEBUG_PRINT_RESPONSE
		Out(SYS_TRK|LOG_DEBUG) << "Data : " << endl;
		Out(SYS_TRK|LOG_DEBUG) << QString(data) << endl;
#endif
		// search for dictionary, there might be random garbage infront of the data
		int i = 0;
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
			for (int i = 0;i < arr.size();i+=6)
			{
				Uint8 buf[6];
				for (int j = 0;j < 6;j++)
					buf[j] = arr[i + j];

				Uint32 ip = ReadUint32(buf,0);
				QString ip_str = QString("%1.%2.%3.%4")
					.arg((ip & 0xFF000000) >> 24)
					.arg((ip & 0x00FF0000) >> 16)
					.arg((ip & 0x0000FF00) >> 8)
					.arg(ip & 0x000000FF);

				addPeer(ip_str,ReadUint16(buf,4));
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
		
		// Check for IPv6 compact peers
		vn = dict->getValue("peers6");
		if (vn && vn->data().getType() == Value::STRING)
		{
			QByteArray arr = vn->data().toByteArray();
			for (int i = 0;i < arr.size();i+=18)
			{
				Uint8 buf[18];
				for (int j = 0;j < 18;j++)
					buf[j] = arr[i + j];

				KNetwork::KIpAddress ip(buf,6);
				addPeer(ip.toString(),ReadUint16(buf,16));
			}
		}
		
		delete n;
		return true;
	}

	
	void HTTPTracker::onAnnounceResult(KJob* j)
	{
		timer.stop();
		KIO::StoredTransferJob* st = (KIO::StoredTransferJob*)j;
		KUrl u = st->url();
		active_job = 0;
		if (j->error())
		{
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
				event = QString();
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
		md["UserAgent"] = bt::GetVersionString();
		md["SendLanguageSettings"] = "false";
		md["cookies"] = "none";
	//	md["accept"] = "text/plain";
		md["accept"] = "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2";
		if (proxy_on)
		{
			QString p = QString("%1:%2").arg(proxy).arg(proxy_port);
			if (!p.startsWith("http://"))
				p = "http://" + p;
			// set the proxy if the doNotUseKDEProxy ix enabled (URL must be valid to)
			KUrl url = KUrl(p);
			if (url.isValid() && proxy.trimmed().length() >  0)
				md["UseProxy"] = p;
			else
				md["UseProxy"] = QString();
			
			Out(SYS_TRK|LOG_DEBUG) << "Using proxy : " << md["UseProxy"] << endl;
		}
	}
	
	void HTTPTracker::doAnnounceQueue()
	{
		if (announce_queue.empty())
			return;
		
		KUrl u = announce_queue.front();
		announce_queue.pop_front();
		doAnnounce(u);
	}
	
	void HTTPTracker::doAnnounce(const KUrl & u)
	{
		Out(SYS_TRK|LOG_NOTICE) << "Doing tracker request to url : " << u.prettyUrl() << endl;
		KIO::MetaData md;
		setupMetaData(md);
		KIO::StoredTransferJob* j = KIO::storedGet(u, KIO::NoReload, KIO::HideProgressInfo);
		// set the meta data
		j->setMetaData(md);
		KIO::Scheduler::scheduleJob(j);
		
		connect(j,SIGNAL(result(KJob* )),this,SLOT(onAnnounceResult( KJob* )));
		
		active_job = j;
		timer.start(60*1000);
		requestPending();
	}
	
	void HTTPTracker::onTimeout() 
	{
		if (active_job)
			active_job->kill(KJob::EmitResult);
	}

	
	void HTTPTracker::setProxy(const QString & p,const bt::Uint16 port) 
	{
		proxy = p;
		proxy_port = port;
	}
	
	void HTTPTracker::setProxyEnabled(bool on) 
	{
		proxy_on = on;
	}
}
#include "httptracker.moc"
