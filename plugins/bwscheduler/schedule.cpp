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
	
	ScheduleItem::ScheduleItem() : day(0),upload_limit(0),download_limit(0),paused(false)
	{
	}
	
	ScheduleItem::ScheduleItem(const ScheduleItem & item)
	{
		operator = (item);
	}
	
	ScheduleItem::ScheduleItem(int day,const QTime & start,const QTime & end,bt::Uint32 upload_limit,bt::Uint32 download_limit,	bool paused)
		: day(day),start(start),end(end),upload_limit(upload_limit),download_limit(download_limit),paused(paused)
	{
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
	
	ScheduleItem & ScheduleItem::operator = (const ScheduleItem & item)
	{
		day = item.day;
		start = item.start;
		end = item.end;
		upload_limit = item.upload_limit;
		download_limit = item.download_limit;
		paused = item.paused;
		return *this;
	}
	
	bool ScheduleItem::operator == (const ScheduleItem & item) const
	{
		return day == item.day &&
				start == item.start &&
				end == item.end &&
				upload_limit == item.upload_limit &&
				download_limit == item.download_limit &&
				paused == item.paused;
	}
	
	/////////////////////////////////////////

	Schedule::Schedule()
	{}


	Schedule::~Schedule()
	{}

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
				
				ScheduleItem item;
				if (parseItem(&item,dict))
					addItem(item);
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
		foreach (ScheduleItem i,*this)
		{
			enc.beginDict();
			enc.write("day"); enc.write((Uint32)i.day);
			enc.write("start"); enc.write(i.start.toString());
			enc.write("end"); enc.write(i.end.toString());
			enc.write("upload_limit"); enc.write(i.upload_limit);
			enc.write("download_limit"); enc.write(i.download_limit);
			enc.write("paused"); enc.write((Uint32) (i.paused ? 1 : 0));
			enc.end();
		}
		enc.end();
	}
	
	bool Schedule::addItem(const ScheduleItem & item)
	{
		if (!item.isValid() || item.end <= item.start)
			return false;
		
		foreach (const ScheduleItem & i,*this)
		{
			if (item.conflicts(i))
				return false;
		}
		
		append(item);
		return true;
	}
}
