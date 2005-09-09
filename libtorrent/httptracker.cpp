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
#include <libutil/log.h>
#include <libutil/functions.h>
#include <libutil/error.h>
#include "bnode.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"
#include "peermanager.h"


namespace bt
{

	HTTPTracker::HTTPTracker() : http(0),cid(0),num_attempts(-1)
	{
		http = new QHttp(this);
		connect(http,SIGNAL(requestFinished(int, bool )),this,SLOT(requestFinished(int, bool )));
		connect(&conn_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
	}


	HTTPTracker::~HTTPTracker()
	{}

	void HTTPTracker::updateData(TorrentControl* tc,PeerManager* pman)
	{
		BDecoder dec(data);
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
			
		Uint32 update_time = vn->data().toInt() > 300 ? 300 : vn->data().toInt();
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
		//if (url.protocol() != "http")
		//	url.setProtocol("http");
		last_url = u;
		KURL url = u;
		

		QString query = QString("&info_hash=") + info_hash.toURLString();
		url.addQueryItem("peer_id",peer_id.toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(uploaded));
		url.addQueryItem("downloaded",QString::number(downloaded));
		url.addQueryItem("left",QString::number(left));
		url.addQueryItem("compact","1");
		url.addQueryItem("numwant","100");

		if (event != QString::null)
			url.addQueryItem("event",event);
		

		Uint16 http_port = url.port();
		if (http_port == 0)
			http_port = 80;

		Out() << "Doing tracker request to url : " << url << endl;
		doRequest(url.host(),url.encodedPathAndQuery() + query,http_port);
		//Out() << "Request " << url << endl;
	}

	void HTTPTracker::doRequest(const QString & host,const QString & path,Uint16 p)
	{
		QHttpRequestHeader header( "GET",path);
		header.setValue( "Host",host );

		http->setHost(host,p);
		cid = http->request(header);
		if (num_attempts < 0)
		{
			num_attempts = 0;
			conn_timer.start(30 * 1000);
		}
	}


	void HTTPTracker::requestFinished(int id,bool err)
	{
		if (cid != id)
			return;
		
		conn_timer.stop();
		num_attempts = -1;

		if (!err)
		{
			data = http->readAll();
			dataReady();
		}
		else
		{
			Out() << "Tracker Error : " << http->errorString() << endl;
			error();
		}
	}

	void HTTPTracker::onTimeout()
	{
		num_attempts++;
		Out() << "Tracker timeout " << num_attempts << endl; 
		if (num_attempts >= 5)
		{
			conn_timer.stop();
			num_attempts = -1;
			error();
		}
		else
		{
			doRequest(last_url);
		}
	}

}
#include "httptracker.moc"
