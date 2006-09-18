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
#include <kio/job.h>
#include <kio/netaccess.h>
#include "bnode.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"
#include "peermanager.h"
#include "server.h"
#include "globals.h"


using namespace kt;

namespace bt
{

	HTTPTracker::HTTPTracker(const KURL & url,kt::TorrentInterface* tor,const PeerID & id)
		: Tracker(url,tor,id)
	{
		active_job = active_scrape_job = 0;
		
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
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
		// time out after 30 seconds
		timer.stop();
		timer.start(30000);
	}
	
	void HTTPTracker::stop()
	{
		if (!started)
			return;
		
		event = "stopped";
		doRequest();
		timer.stop();
		started = false;
	}
	
	void HTTPTracker::completed()
	{
		event = "completed";
		doRequest();
		event = QString::null;
		// time out after 30 seconds
		timer.stop();
		timer.start(30000);
	}
	
	void HTTPTracker::manualUpdate()
	{
		if (!started)
			event = "started";
		doRequest();
		// time out after 30 seconds
		timer.stop();
		timer.start(30000);
	}
	
	void HTTPTracker::scrape()
	{
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
			epq += "&infohash=" + info_hash.toURLString();
		else
			epq += "?infohash=" + info_hash.toURLString();
		scrape_url.setEncodedPathAndQuery(epq);
	
		Out(SYS_TRK|LOG_NOTICE) << "Doing scrape request to url : " << scrape_url.prettyURL() << endl;
		KIO::MetaData md;
		md["UserAgent"] = "ktorrent/" VERSION;
		md["SendLanguageSettings"] = "false";
		md["Cookies"] = "none";
		
		KIO::StoredTransferJob* j = KIO::storedGet(scrape_url,false,false);
		// set the meta data
		j->setMetaData(md);
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onScrapeResult( KIO::Job* )));
		active_scrape_job = j;
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
	
	void HTTPTracker::doRequest()
	{	
		const TorrentStats & s = tor->getStats();
		
		KURL u = url;

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
		if (!Tracker::custom_ip_resolved.isNull())
			u.addQueryItem("ip",Tracker::custom_ip_resolved);

		if (event != QString::null)
			u.addQueryItem("event",event);
		QString epq = u.encodedPathAndQuery();
		const SHA1Hash & info_hash = tor->getInfoHash();
		epq += "&info_hash=" + info_hash.toURLString();


		u.setEncodedPathAndQuery(epq);
	
		Out(SYS_TRK|LOG_NOTICE) << "Doing tracker request to url : " << u.prettyURL() << endl;

		KIO::MetaData md;
		md["UserAgent"] = "ktorrent/" VERSION;
		md["SendLanguageSettings"] = "false";
		md["Cookies"] = "none";
		
		KIO::StoredTransferJob* j = KIO::storedGet(u,false,false);
		// set the meta data
		j->setMetaData(md);
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onAnnounceResult( KIO::Job* )));
		
		active_job = j;
		requestPending();
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
		if (j != active_job)
		{
			return;
		}
		
		if (j->error())
		{
			Out(SYS_TRK|LOG_IMPORTANT) << "Error : " << j->errorString() << endl;
			active_job = 0;
			
			timer.stop();
			if (event != "stopped")
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
			timer.stop();
			KIO::StoredTransferJob* st = (KIO::StoredTransferJob*)active_job;
			failures = 0;
			active_job = 0;
			if (event != "stopped")
			{
				if (event == "started")
					started = true;
				
				event = QString::null;
				try
				{
					if (updateData(st->data()))
					{
						peersReady(this);
						requestOK();
					}
				}
				catch (bt::Error & err)
				{
					failures++;
					requestFailed(i18n("Invalid response from tracker"));
				}
			}
			else
			{
				stopDone();
			}
		}
	}

	void HTTPTracker::onTimeout()
	{
		if (active_job)
		{
			// current job timed out kill it
			active_job->kill();
			active_job = 0;
			failures++;
			requestFailed(i18n("Tracker request timed out"));
			
			// try again 
			if (event != "stopped" && started)
			{
				failures++;
				timer.stop();
				if (failures < 5)
					timer.start(30000);
				else
					timer.start(5 * 60 * 1000);
			}
			else
			{
				timer.stop();
				stopDone();
			}
		}
		else
		{
			if (event != "stopped" && started)
			{
				// do a new request
				doRequest();
				timer.stop();
				// timeout in 30 seconds
				timer.start(30000);
			}
			else
			{
				timer.stop();
			}
		}
	}
}
#include "httptracker.moc"
