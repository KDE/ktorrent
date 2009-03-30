/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#include <QFile>
#include <util/file.h>
#include <util/error.h>
#include <util/log.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include "schedule.h"

using namespace bt;

namespace kt
{
	
	ScheduleItem::ScheduleItem() 
		: day(0),upload_limit(0),download_limit(0),paused(false),set_conn_limits(false),global_conn_limit(0),torrent_conn_limit(0)
	{
		screensaver_limits = false;
		ss_download_limit = ss_upload_limit = 0;
	}
	
	ScheduleItem::ScheduleItem(const ScheduleItem & item)
	{
		operator = (item);
	}

	bool ScheduleItem::conflicts(const ScheduleItem & other) const
	{
		if (day != other.day)
			return false;
		else if (other.start >= start && other.start <= end)
			return true;
		else if (other.end >= start && other.end <= end)
			return true;
		else
			return false;
	}
	
	bool ScheduleItem::contains(const QDateTime & dt) const
	{
		if (dt.date().dayOfWeek() != day)
			return false;
		else
			return start <= dt.time() && dt.time() <= end;
	}
	
	ScheduleItem & ScheduleItem::operator = (const ScheduleItem & item)
	{
		day = item.day;
		start = item.start;
		end = item.end;
		upload_limit = item.upload_limit;
		download_limit = item.download_limit;
		paused = item.paused;
		screensaver_limits = item.screensaver_limits;
		ss_download_limit = item.ss_download_limit;
		ss_upload_limit = item.ss_upload_limit;
		set_conn_limits = item.set_conn_limits;
		global_conn_limit = item.global_conn_limit;
		torrent_conn_limit = item.torrent_conn_limit;
		return *this;
	}
	
	bool ScheduleItem::operator == (const ScheduleItem & item) const
	{
		return day == item.day &&
				start == item.start &&
				end == item.end &&
				upload_limit == item.upload_limit &&
				download_limit == item.download_limit &&
				paused == item.paused && 
				set_conn_limits == item.set_conn_limits &&
				global_conn_limit == item.global_conn_limit &&
				torrent_conn_limit == item.torrent_conn_limit &&
				screensaver_limits == item.screensaver_limits &&
				ss_download_limit == item.ss_download_limit &&
				ss_upload_limit == item.ss_upload_limit;
	}
	
	
	/////////////////////////////////////////

	Schedule::Schedule()
	{}


	Schedule::~Schedule()
	{
		qDeleteAll(*this);
	}

	void Schedule::load(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			QString msg = i18n("Cannot open file %1: %2",file,fptr.errorString());
			Out(SYS_SCD|LOG_NOTICE) << msg << endl;
			throw bt::Error(msg);
		}
		
		QByteArray data = fptr.readAll();
		BDecoder dec(data,false,0);
		BNode* node = 0;
		try
		{
			node = dec.decode();
		}
		catch (bt::Error & err)
		{
			delete node;
			Out(SYS_SCD|LOG_NOTICE) << "Decoding " << file << " failed : " << err.toString() << endl;
			throw bt::Error(i18n("The file %1 is corrupted or not a proper KTorrent schedule file !"));
		}
		
		if (!node)
		{
			Out(SYS_SCD|LOG_NOTICE) << "Decoding " << file << " failed !" << endl;
			throw bt::Error(i18n("The file %1 is corrupted or not a proper KTorrent schedule file !"));
		}
		
		if (node->getType() == BNode::LIST)
		{
			BListNode* ln = (BListNode*)node;
			for (Uint32 i = 0;i < ln->getNumChildren();i++)
			{
				BDictNode* dict = ln->getDict(i);
				if (!dict)
					continue;
				
				ScheduleItem* item = new ScheduleItem();
				if (parseItem(item,dict))
					addItem(item);
				else
					delete item;
			}
		}
		
		delete node;
	}
	
	bool Schedule::parseItem(ScheduleItem* item,bt::BDictNode* dict)
	{
		BValueNode* day = dict->getValue("day");
		BValueNode* start = dict->getValue("start");
		BValueNode* end = dict->getValue("end");
		BValueNode* upload_limit = dict->getValue("upload_limit");
		BValueNode* download_limit = dict->getValue("download_limit");
		BValueNode* paused = dict->getValue("paused");
		
		if (!day || !start || !end || !upload_limit || !download_limit || !paused)
			return false;
		
		item->day = day->data().toInt();
		item->start = QTime::fromString(start->data().toString());
		item->end = QTime::fromString(end->data().toString());
		item->upload_limit = upload_limit->data().toInt();
		item->download_limit = download_limit->data().toInt();
		item->paused = paused->data().toInt() == 1;
		item->set_conn_limits = false;
		
		BDictNode* conn_limits = dict->getDict(QString("conn_limits"));
		if (conn_limits)
		{
			BValueNode* glob = conn_limits->getValue("global");
			BValueNode* per_torrent = conn_limits->getValue("per_torrent");
			if (glob && per_torrent)
			{
				item->global_conn_limit = glob->data().toInt();
				item->torrent_conn_limit = per_torrent->data().toInt();
				item->set_conn_limits = true;
			}
		}
		
		BValueNode* ss_limits = dict->getValue(QString("screensaver_limits"));
		if (ss_limits)
		{
			item->screensaver_limits = ss_limits->data().toInt() == 1;
			item->ss_download_limit = dict->getInt("ss_download_limit");
			item->ss_upload_limit = dict->getInt("ss_upload_limit");
		}
		else
		{
			item->screensaver_limits = false;
			item->ss_download_limit = item->ss_upload_limit = 0;
		}
		
		return true;
	}
		
	void Schedule::save(const QString & file)
	{
		File fptr;
		if (!fptr.open(file,"wb"))
		{
			QString msg = i18n("Cannot open file %1: %2",file,fptr.errorString());
			Out(SYS_SCD|LOG_NOTICE) << msg << endl;
			throw bt::Error(msg);
		}

		BEncoder enc(&fptr);
		enc.beginList();
		for (iterator itr = begin();itr != end();itr++)
		{
			ScheduleItem* i = *itr;
			enc.beginDict();
			enc.write("day"); enc.write((Uint32)i->day);
			enc.write("start"); enc.write(i->start.toString());
			enc.write("end"); enc.write(i->end.toString());
			enc.write("upload_limit"); enc.write(i->upload_limit);
			enc.write("download_limit"); enc.write(i->download_limit);
			enc.write("paused"); enc.write((Uint32) (i->paused ? 1 : 0));
			if (i->set_conn_limits)
			{
				enc.write("conn_limits"); 
				enc.beginDict();
				enc.write("global"); enc.write((Uint32)i->global_conn_limit);
				enc.write("per_torrent"); enc.write((Uint32)i->torrent_conn_limit);
				enc.end();
			}
			enc.write("screensaver_limits",(Uint32)i->screensaver_limits);
			enc.write("ss_upload_limit",i->ss_upload_limit);
			enc.write("ss_download_limit",i->ss_download_limit);
			enc.end();
		}
		enc.end();
	}
	
	bool Schedule::addItem(ScheduleItem* item)
	{
		if (!item->isValid() || item->end <= item->start)
			return false;
		
		for (iterator itr = begin();itr != end();itr++)
		{
			ScheduleItem* i = *itr;
			if (item->conflicts(*i))
				return false;
		}
		
		append(item);
		return true;
	}
	
	ScheduleItem* Schedule::getCurrentItem(const QDateTime & now)
	{
		for (iterator itr = begin();itr != end();itr++)
		{
			ScheduleItem* i = *itr;
			if (i->contains(now))
			{
				return i;
			}
		}
		return 0;
	}
	
	int Schedule::getTimeToNextScheduleEvent(const QDateTime & now)
	{
		ScheduleItem* item = getCurrentItem(now);
		// when we are in the middle of a ScheduleItem, we need to trigger again at the end of it
		if (item) 
			return now.time().secsTo(item->end) + 1; // change the schedule one second after it expires
		
		// lets look at all schedule items on the same day
		// and find the next one
		for (iterator itr = begin();itr != end();itr++)
		{
			ScheduleItem* i = *itr;
			if (i->day == now.date().dayOfWeek() && i->start > now.time())
			{
				if (!item || i->start < item->start)
					item = i;
			}
		}
		
		if (item)
			return now.time().secsTo(item->start);
		
		QTime end_of_day(23,59,59);
		return now.time().secsTo(end_of_day) + 1;
	}
	
	bool Schedule::modify(ScheduleItem* item,const QTime & start,const QTime & end,int day)
	{
		QTime old_start = item->start;
		QTime old_end = item->end;
		int old_day = item->day;
			
		item->start = start;
		item->end = end;
		item->day = day;
		if (conflicts(item))
		{
			// restore old start and end time
			item->start = old_start;
			item->end = old_end;
			item->day = old_day;
			return false;
		}
	
		return true;
	}
	
	bool Schedule::conflicts(ScheduleItem* item)
	{
		for (iterator itr = begin();itr != end();itr++)
		{
			ScheduleItem* i = *itr;
			if (i != item && (i->conflicts(*item) || item->conflicts(*i)))
				return true;
		}
		return false;
	}
}
