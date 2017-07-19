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

#include "weekview.h"

#include <QGraphicsItem>

#include <util/log.h>
#include <boost/bind.hpp>
#include "weekscene.h"
#include "schedule.h"

using namespace bt;

namespace kt
{

    WeekView::WeekView(QWidget* parent) : QGraphicsView(parent), schedule(nullptr)
    {
        scene = new WeekScene(this);
        setScene(scene);

        connect(scene, &WeekScene::selectionChanged, this, &WeekView::onSelectionChanged);
        connect(scene, &WeekScene::itemDoubleClicked, this, &WeekView::onDoubleClicked);
        connect(scene, static_cast<void (WeekScene::*)(ScheduleItem*, const QTime&, const QTime&, int, int)>(&WeekScene::itemMoved), this, &WeekView::itemMoved);

        menu = new QMenu(this);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &WeekView::customContextMenuRequested, this, &WeekView::showContextMenu);
    }


    WeekView::~WeekView()
    {
    }

    void WeekView::updateStatusText(int up, int down, bool suspended, bool enabled)
    {
        scene->updateStatusText(up, down, suspended, enabled);
    }

    void WeekView::onSelectionChanged()
    {
        selection.clear();

        const QList<QGraphicsItem*> sel = scene->selectedItems();
        for (QGraphicsItem* s : sel)
        {
            QMap<QGraphicsItem*, ScheduleItem*>::iterator i = item_map.find(s);
            if (i != item_map.end())
                selection.append(i.value());
        }

        selectionChanged();
    }

    void WeekView::setSchedule(Schedule* s)
    {
        clear();
        schedule = s;

        if (schedule)
            s->apply(boost::bind(&WeekView::addScheduleItem, this, _1));

        scene->setSchedule(s);
    }

    void WeekView::clear()
    {
        QMap<QGraphicsItem*, ScheduleItem*>::iterator i = item_map.begin();
        while (i != item_map.end())
        {
            QGraphicsItem* item = i.key();
            scene->removeItem(item);
            delete item;
            i++;
        }
        item_map.clear();
        selection.clear();
        schedule = 0;
    }

    void WeekView::removeSelectedItems()
    {
        QList<QGraphicsItem*> sel = scene->selectedItems();
        foreach (QGraphicsItem* s, sel)
        {
            QMap<QGraphicsItem*, ScheduleItem*>::iterator i = item_map.find(s);
            if (i != item_map.end())
            {
                ScheduleItem* si = i.value();
                scene->removeItem(s);
                item_map.erase(i);
                schedule->removeItem(si);
            }
        }
    }

    void WeekView::addScheduleItem(ScheduleItem* item)
    {
        QGraphicsItem* gi = scene->addScheduleItem(item);

        if (gi)
            item_map[gi] = item;
    }

    void WeekView::onDoubleClicked(QGraphicsItem* i)
    {
        QMap<QGraphicsItem*, ScheduleItem*>::iterator itr = item_map.find(i);
        if (itr != item_map.end())
            editItem(itr.value());
    }

    void WeekView::showContextMenu(const QPoint& pos)
    {
        menu->popup(viewport()->mapToGlobal(pos));
    }

    void WeekView::itemChanged(ScheduleItem* item)
    {
        QMap<QGraphicsItem*, ScheduleItem*>::iterator i = item_map.begin();
        while (i != item_map.end())
        {
            if (item == i.value())
            {
                QGraphicsItem* gi = i.key();
                scene->itemChanged(item, gi);
                break;
            }
            i++;
        }
    }

    void WeekView::colorsChanged()
    {
        scene->colorsChanged();
    }
}

