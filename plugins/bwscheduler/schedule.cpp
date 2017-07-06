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

#include <KLocalizedString>
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
        : start_day(0)
        , end_day(0)
        , upload_limit(0)
        , download_limit(0)
        , suspended(false)
        , screensaver_limits(false)
        , ss_upload_limit(0)
        , ss_download_limit(0)
        , set_conn_limits(false)
        , global_conn_limit(0)
        , torrent_conn_limit(0)
    {
    }

    ScheduleItem::ScheduleItem(const ScheduleItem& item)
    {
        operator = (item);
    }

    bool ScheduleItem::conflicts(const ScheduleItem& other) const
    {
        bool on_same_day = between(other.start_day, start_day, end_day) ||
                           between(other.end_day, start_day, end_day) ||
                           (other.start_day <= start_day && other.end_day >= end_day);

        return on_same_day &&
            (  between(other.start, start, end)
            || between(other.end, start, end)
            || (other.start <= start && other.end >= end)
            );
    }

    bool ScheduleItem::contains(const QDateTime& dt) const
    {
        return between(dt.date().dayOfWeek(), start_day, end_day)
            && between(dt.time(), start, end);
    }

    ScheduleItem& ScheduleItem::operator = (const ScheduleItem& item)
    {
        start_day = item.start_day;
        end_day = item.end_day;
        start = item.start;
        end = item.end;
        upload_limit = item.upload_limit;
        download_limit = item.download_limit;
        suspended = item.suspended;
        screensaver_limits = item.screensaver_limits;
        ss_download_limit = item.ss_download_limit;
        ss_upload_limit = item.ss_upload_limit;
        set_conn_limits = item.set_conn_limits;
        global_conn_limit = item.global_conn_limit;
        torrent_conn_limit = item.torrent_conn_limit;
        return *this;
    }

    bool ScheduleItem::operator == (const ScheduleItem& item) const
    {
        return start_day == item.start_day &&
               end_day == item.end_day &&
               start == item.start &&
               end == item.end &&
               upload_limit == item.upload_limit &&
               download_limit == item.download_limit &&
               suspended == item.suspended &&
               set_conn_limits == item.set_conn_limits &&
               global_conn_limit == item.global_conn_limit &&
               torrent_conn_limit == item.torrent_conn_limit &&
               screensaver_limits == item.screensaver_limits &&
               ss_download_limit == item.ss_download_limit &&
               ss_upload_limit == item.ss_upload_limit;
    }

    void ScheduleItem::checkTimes()
    {
        start.setHMS(start.hour(), start.minute(), 0);
        end.setHMS(end.hour(), end.minute(), 59);
    }


    /////////////////////////////////////////

    Schedule::Schedule() : enabled(true)
    {}


    Schedule::~Schedule()
    {
        qDeleteAll(items);
    }

    void Schedule::load(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            QString msg = i18n("Cannot open file %1: %2", file, fptr.errorString());
            Out(SYS_SCD | LOG_NOTICE) << msg << endl;
            throw bt::Error(msg);
        }

        QByteArray data = fptr.readAll();
        BDecoder dec(data, false, 0);
        BNode* node = 0;
        try
        {
            node = dec.decode();
        }
        catch (bt::Error& err)
        {
            delete node;
            Out(SYS_SCD | LOG_NOTICE) << "Decoding " << file << " failed : " << err.toString() << endl;
            throw bt::Error(i18n("The file %1 is corrupted or not a proper KTorrent schedule file.", file));
        }

        if (!node)
        {
            Out(SYS_SCD | LOG_NOTICE) << "Decoding " << file << " failed !" << endl;
            throw bt::Error(i18n("The file %1 is corrupted or not a proper KTorrent schedule file.", file));
        }

        if (node->getType() == BNode::LIST)
        {
            // Old format
            parseItems((BListNode*)node);
        }
        else if (node->getType() == BNode::DICT)
        {
            BDictNode* dict = (BDictNode*)node;
            BListNode* items = dict->getList(QByteArrayLiteral("items"));
            if (items)
                parseItems(items);

            try
            {
                enabled = dict->getInt(QByteArrayLiteral("enabled")) == 1;
            }
            catch (...)
            {
                enabled = true;
            }
        }

        delete node;
    }

    void Schedule::parseItems(BListNode* items)
    {
        for (Uint32 i = 0; i < items->getNumChildren(); i++)
        {
            BDictNode* dict = items->getDict(i);
            if (!dict)
                continue;

            ScheduleItem* item(new ScheduleItem());
            if (parseItem(item, dict))
                addItem(item);
            else
                delete item;
        }
    }

    bool Schedule::parseItem(ScheduleItem* item, bt::BDictNode* dict)
    {
        // Must have at least a day or days entry
        BValueNode* day = dict->getValue(QByteArrayLiteral("day"));
        BValueNode* start_day = dict->getValue(QByteArrayLiteral("start_day"));
        BValueNode* end_day = dict->getValue(QByteArrayLiteral("end_day"));
        if (!day && !start_day && !end_day)
            return false;

        BValueNode* start = dict->getValue(QByteArrayLiteral("start"));
        BValueNode* end = dict->getValue(QByteArrayLiteral("end"));
        BValueNode* upload_limit = dict->getValue(QByteArrayLiteral("upload_limit"));
        BValueNode* download_limit = dict->getValue(QByteArrayLiteral("download_limit"));
        BValueNode* suspended = dict->getValue(QByteArrayLiteral("suspended"));

        if (!start || !end || !upload_limit || !download_limit || !suspended)
            return false;

        if (day)
            item->start_day = item->end_day = day->data().toInt();
        else
        {
            item->start_day = start_day->data().toInt();
            item->end_day = end_day->data().toInt();
        }


        item->start = QTime::fromString(start->data().toString());
        item->end = QTime::fromString(end->data().toString());
        item->upload_limit = upload_limit->data().toInt();
        item->download_limit = download_limit->data().toInt();
        item->suspended = suspended->data().toInt() == 1;
        item->set_conn_limits = false;

        BDictNode* conn_limits = dict->getDict(QByteArrayLiteral("conn_limits"));
        if (conn_limits)
        {
            BValueNode* glob = conn_limits->getValue(QByteArrayLiteral("global"));
            BValueNode* per_torrent = conn_limits->getValue(QByteArrayLiteral("per_torrent"));
            if (glob && per_torrent)
            {
                item->global_conn_limit = glob->data().toInt();
                item->torrent_conn_limit = per_torrent->data().toInt();
                item->set_conn_limits = true;
            }
        }

        BValueNode* ss_limits = dict->getValue(QByteArrayLiteral("screensaver_limits"));
        if (ss_limits)
        {
            item->screensaver_limits = ss_limits->data().toInt() == 1;
            item->ss_download_limit = dict->getInt(QByteArrayLiteral("ss_download_limit"));
            item->ss_upload_limit = dict->getInt(QByteArrayLiteral("ss_upload_limit"));
        }
        else
        {
            item->screensaver_limits = false;
            item->ss_download_limit = item->ss_upload_limit = 0;
        }

        item->checkTimes();
        return true;
    }

    void Schedule::save(const QString& file)
    {
        File fptr;
        if (!fptr.open(file, QStringLiteral("wb")))
        {
            QString msg = i18n("Cannot open file %1: %2", file, fptr.errorString());
            Out(SYS_SCD | LOG_NOTICE) << msg << endl;
            throw bt::Error(msg);
        }

        BEncoder enc(&fptr);
        enc.beginDict();
        enc.write(QByteArrayLiteral("enabled"), enabled);
        enc.write(QByteArrayLiteral("items"));
        enc.beginList();
        for (ScheduleItem* i : qAsConst(items))
        {
            enc.beginDict();
            enc.write(QByteArrayLiteral("start_day")); enc.write((Uint32)i->start_day);
            enc.write(QByteArrayLiteral("end_day")); enc.write((Uint32)i->end_day);
            enc.write(QByteArrayLiteral("start")); enc.write(i->start.toString().toLatin1());
            enc.write(QByteArrayLiteral("end")); enc.write(i->end.toString().toLatin1());
            enc.write(QByteArrayLiteral("upload_limit")); enc.write(i->upload_limit);
            enc.write(QByteArrayLiteral("download_limit")); enc.write(i->download_limit);
            enc.write(QByteArrayLiteral("suspended")); enc.write((Uint32)(i->suspended ? 1 : 0));
            if (i->set_conn_limits)
            {
                enc.write(QByteArrayLiteral("conn_limits"));
                enc.beginDict();
                enc.write(QByteArrayLiteral("global")); enc.write((Uint32)i->global_conn_limit);
                enc.write(QByteArrayLiteral("per_torrent")); enc.write((Uint32)i->torrent_conn_limit);
                enc.end();
            }
            enc.write(QByteArrayLiteral("screensaver_limits"), (Uint32)i->screensaver_limits);
            enc.write(QByteArrayLiteral("ss_upload_limit"), i->ss_upload_limit);
            enc.write(QByteArrayLiteral("ss_download_limit"), i->ss_download_limit);
            enc.end();
        }
        enc.end();
        enc.end();
    }

    void Schedule::clear()
    {
        qDeleteAll(items);
        items.clear();
    }


    bool Schedule::addItem(ScheduleItem* item)
    {
        if (!item->isValid() || item->end <= item->start)
            return false;

        for (ScheduleItem* i : qAsConst(items))
        {
            if (item->conflicts(*i))
                return false;
        }

        items.append(item);
        return true;
    }

    void Schedule::removeItem(ScheduleItem* item)
    {
        if (items.removeAll(item) > 0)
            delete item;
    }


    ScheduleItem* Schedule::getCurrentItem(const QDateTime& now)
    {
        for (ScheduleItem* i : qAsConst(items))
        {
            if (i->contains(now))
            {
                return i;
            }
        }
        return 0;
    }

    int Schedule::getTimeToNextScheduleEvent(const QDateTime& now)
    {
        ScheduleItem* item = getCurrentItem(now);
        // when we are in the middle of a ScheduleItem, we need to trigger again at the end of it
        if (item)
            return now.time().secsTo(item->end) + 5; // change the schedule 5 seconds after it expires

        // lets look at all schedule items on the same day
        // and find the next one
        for (ScheduleItem* i : qAsConst(items))
        {
            if (between(now.date().dayOfWeek(), i->start_day, i->end_day) && i->start > now.time())
            {
                if (!item || i->start < item->start)
                    item = i;
            }
        }

        if (item)
            return now.time().secsTo(item->start) + 5;

        QTime end_of_day(23, 59, 59);
        return now.time().secsTo(end_of_day) + 5;
    }

    bool Schedule::modify(kt::ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day)
    {
        QTime old_start = item->start;
        QTime old_end = item->end;
        int old_start_day = item->start_day;
        int old_end_day = item->end_day;

        item->start = start;
        item->end = end;
        item->start_day = start_day;
        item->end_day = end_day;
        item->checkTimes();
        if (!item->isValid() || conflicts(item))
        {
            // restore old start and end time
            item->start = old_start;
            item->end = old_end;
            item->start_day = old_start_day;
            item->end_day = old_end_day;
            return false;
        }

        return true;
    }

    bool Schedule::validModify(ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day)
    {
        QTime old_start = item->start;
        QTime old_end = item->end;
        int old_start_day = item->start_day;
        int old_end_day = item->end_day;

        item->start = start;
        item->end = end;
        item->start_day = start_day;
        item->end_day = end_day;
        item->checkTimes();
        bool invalid = !item->isValid() || conflicts(item);

        // restore old start and end time
        item->start = old_start;
        item->end = old_end;
        item->start_day = old_start_day;
        item->end_day = old_end_day;
        return !invalid;
    }


    bool Schedule::conflicts(ScheduleItem* item)
    {
        for (ScheduleItem* i : qAsConst(items))
        {
            if (i != item && (i->conflicts(*item) || item->conflicts(*i)))
                return true;
        }
        return false;
    }

    void Schedule::setEnabled(bool on)
    {
        enabled = on;
    }

}
