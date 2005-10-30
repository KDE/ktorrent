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
#include <kurl.h>
#include <klocale.h>
#include <qhostaddress.h>
#include <util/log.h>
#include <util/functions.h>
#include <util/error.h>
#include <kio/job.h>
#include "bnode.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"
#include "peermanager.h"


namespace bt
{

	HTTPTracker::HTTPTracker() : num_attempts(-1)
	{
		active_job = 0;
	}


	HTTPTracker::~HTTPTracker()
	{}

	void HTTPTracker::updateData(TorrentControl* tc,PeerManager* pman)
	{
		BDecoder dec(data,false);
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
			
		Uint32 update_time = vn->data().toInt();
		tc->setTrackerTimerInterval(update_time * 1000);

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

				if (!ip_node || !port_node || !id_node)
					continue;
				
				PotentialPeer pp;
				pp.ip = ip_node->data().toString();
				pp.port = port_node->data().toInt();
				pp.id = PeerID(id_node->data().toByteArray().data());
				pman->addPotentialPeer(pp);
			}
		}
		delete n;
	}

	void HTTPTracker::doRequest(const KURL & u)
	{	
		// clear data array
		data = QByteArray();
		last_url = u;
		KURL url = u;
		
		url.addQueryItem("peer_id",peer_id.toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(uploaded));
		url.addQueryItem("downloaded",QString::number(downloaded));
		url.addQueryItem("left",QString::number(left));
		url.addQueryItem("compact","1");
		url.addQueryItem("numwant","100");

		if (event != QString::null)
			url.addQueryItem("event",event);
		QString epq = url.encodedPathAndQuery();
		epq += "&info_hash=" + info_hash.toURLString();
		url.setEncodedPathAndQuery(epq);
	//	Out() << "query : " << url.query() << endl;
		Out() << "Doing tracker request to url : " << url.prettyURL() << endl;
		KIO::TransferJob* j = KIO::get(url,true,false);
		j->addMetaData("User-Agent","ktorrent");
		
		connect(j,SIGNAL(result(KIO::Job* )),this,SLOT(onResult(KIO::Job* )));
		connect(j,SIGNAL(data(KIO::Job*,const QByteArray &)),
				this,SLOT(onDataRecieved(KIO::Job*, const QByteArray& )));
		active_job = j;
	}

	void HTTPTracker::onResult(KIO::Job* j)
	{
		if (j != active_job)
			return;
		
		if (j->error())
		{
			Out() << "Error : " << j->errorString() << endl;
			active_job = 0;
			error();
		}
		else
		{
			active_job = 0;
			dataReady();
		}
	}
	
	void HTTPTracker::onDataRecieved(KIO::Job* j,const QByteArray & ba)
	{
		if (j != active_job)
			return;

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
		error();
	}

}
#include "httptracker.moc"
