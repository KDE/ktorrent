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

	HTTPTracker::HTTPTracker(Tracker* trk) : TrackerBackend(trk)
	{
		active_job = 0;
		
	}


	HTTPTracker::~HTTPTracker()
	{}

	void HTTPTracker::updateData(PeerManager* pman)
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
			throw Error(i18n("Parse Error"));
		
		BDecoder dec(data,false,i);
		BNode* n = dec.decode();
			
		if (!n || n->getType() != BNode::DICT)
			throw Error(i18n("Parse Error"));
			
		BDictNode* dict = (BDictNode*)n;
		if (dict->getData("failure reason"))
		{
			BValueNode* vn = dict->getValue("failure reason");
			QString msg = vn->data().toString();
			delete n;
			throw Error(msg);
		}
			
		BValueNode* vn = dict->getValue("interval");
			
		if (!vn)
		{
			delete n;
			throw Error(i18n("Parse Error"));
		}
			
		frontend->setInterval(vn->data().toInt());

		vn = dict->getValue("incomplete");
		if (vn)
			frontend->leechers = vn->data().toInt();

		vn = dict->getValue("complete");
		if (vn)
			frontend->seeders = vn->data().toInt();
	
		BListNode* ln = dict->getList("peers");
		if (!ln)
		{
			// no list, it might however be a compact response
			vn = dict->getValue("peers");
			if (!vn)
			{
				delete n;
				throw Error(i18n("Parse error"));
			}

			QByteArray arr = vn->data().toByteArray();
			for (Uint32 i = 0;i < arr.size();i+=6)
			{
				Uint8 buf[6];
				for (int j = 0;j < 6;j++)
					buf[j] = arr[i + j];

				PotentialPeer pp;
				pp.ip = QHostAddress(ReadUint32(buf,0)).toString();
				pp.port = ReadUint16(buf,4);
				pman->addPotentialPeer(pp);
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
				BValueNode* id_node = dict->getValue("peer id");

				if (!ip_node || !port_node)
					continue;
				
				PotentialPeer pp;
				pp.ip = ip_node->data().toString();
				pp.port = port_node->data().toInt();
				if (id_node)
					pp.id = PeerID(id_node->data().toByteArray().data());
				pman->addPotentialPeer(pp);
			}
		}
		
		/*
		PotentialPeer pp;
		pp.ip = "127.0.0.1";
		pp.port = 5555;
		pman->addPotentialPeer(pp);
		*/
		delete n;
		frontend->updateOK();
	}

	bool HTTPTracker::doRequest(const KURL & u)
	{	
		// clear data array
		data = QByteArray();
		
		const TorrentStats & s = frontend->tor->getStats();
		last_url = u;
		KURL url = u;

		Uint16 port = Globals::instance().getServer().getPortInUse();;
		
		url.addQueryItem("peer_id", frontend->peer_id.toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(s.trk_bytes_uploaded));
		url.addQueryItem("downloaded",QString::number(s.trk_bytes_downloaded));
		
		if (frontend->event == "completed")
			url.addQueryItem("left","0"); // need to send 0 when we are completed
		else
			url.addQueryItem("left",QString::number(s.bytes_left));
		
		url.addQueryItem("compact","1");
		if (frontend->event != "stopped")
			url.addQueryItem("numwant","100");
		else
			url.addQueryItem("numwant","0");
		url.addQueryItem("key",QString::number(frontend->key));
		if (!Tracker::custom_ip_resolved.isNull())
			url.addQueryItem("ip",Tracker::custom_ip_resolved);

		if (frontend->event != QString::null)
			url.addQueryItem("event",frontend->event);
		QString epq = url.encodedPathAndQuery();
		epq += "&info_hash=" + frontend->info_hash.toURLString();


		url.setEncodedPathAndQuery(epq);
	//	Out() << "query : " << url.query() << endl;
		Out(SYS_TRK|LOG_NOTICE) << "Doing tracker request to url : " << url.prettyURL() << endl;

		
		
		KIO::MetaData md;
		md["UserAgent"] = "ktorrent/" VERSION;
		md["SendLanguageSettings"] = "false";
		md["Cookies"] = "none";
		
		KIO::TransferJob* j = KIO::get(url,true,false);
		// set the meta data
		j->setMetaData(md);
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onResult(KIO::Job* )));
		connect(j,SIGNAL(data(KIO::Job*,const QByteArray &)),
				this,SLOT(onDataRecieved(KIO::Job*, const QByteArray& )));
		active_job = j;
	//	if (event == "stopped")
	//		KIO::NetAccess::synchronousRun(active_job,0);
		return true;
	}

	void HTTPTracker::onResult(KIO::Job* j)
	{
		if (j != active_job)
		{
			return;
		}
		
		if (j->error())
		{
			Out(SYS_TRK|LOG_IMPORTANT) << "Error : " << j->errorString() << endl;
			active_job = 0;
			frontend->emitError();
		}
		else
		{
			active_job = 0;
			frontend->emitDataReady();
		}
	}
	
	void HTTPTracker::onDataRecieved(KIO::Job* j,const QByteArray & ba)
	{
		if (j != active_job)
		{
			return;
		}

		if (ba.size() > 0)
		{
			Uint32 old_size = data.size();
			data.resize(data.size() + ba.size());
			for (Uint32 i = old_size;i < data.size();i++)
				data[i] = ba[i - old_size];
		}
	}

	void HTTPTracker::onTimeout()
	{
		frontend->emitError();
	}
}
#include "httptracker.moc"
