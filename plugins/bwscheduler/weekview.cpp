/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "weekview.h"

#include <QGraphicsItem>

#include "schedule.h"
#include "weekscene.h"
#include <boost/bind/bind.hpp>
#include <util/log.h>

using namespace bt;

namespace kt
{
WeekView::WeekView(QWidget *parent)
    : QGraphicsView(parent)
    , schedule(nullptr)
{
    scene = new WeekScene(this);
    setScene(scene);

    connect(scene, &WeekScene::selectionChanged, this, &WeekView::onSelectionChanged);
    connect(scene, &WeekScene::itemDoubleClicked, this, &WeekView::onDoubleClicked);
    connect(scene, qOverload<ScheduleItem *, const QTime &, const QTime &, int, int>(&WeekScene::itemMoved), this, &WeekView::itemMoved);

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

    const QList<QGraphicsItem *> sel = scene->selectedItems();
    for (QGraphicsItem *s : sel) {
        QMap<QGraphicsItem *, ScheduleItem *>::iterator i = item_map.find(s);
        if (i != item_map.end())
            selection.append(i.value());
    }

    Q_EMIT selectionChanged();
}

void WeekView::setSchedule(Schedule *s)
{
    clear();
    schedule = s;

    if (schedule)
        s->apply(boost::bind(&WeekView::addScheduleItem, this, boost::placeholders::_1));

    scene->setSchedule(s);
}

void WeekView::clear()
{
    QMap<QGraphicsItem *, ScheduleItem *>::iterator i = item_map.begin();
    while (i != item_map.end()) {
        QGraphicsItem *item = i.key();
        scene->removeItem(item);
        delete item;
        i++;
    }
    item_map.clear();
    selection.clear();
    schedule = nullptr;
}

void WeekView::removeSelectedItems()
{
    const QList<QGraphicsItem *> sel = scene->selectedItems();
    for (QGraphicsItem *s : sel) {
        QMap<QGraphicsItem *, ScheduleItem *>::iterator i = item_map.find(s);
        if (i != item_map.end()) {
            ScheduleItem *si = i.value();
            scene->removeItem(s);
            item_map.erase(i);
            schedule->removeItem(si);
        }
    }
}

void WeekView::addScheduleItem(ScheduleItem *item)
{
    QGraphicsItem *gi = scene->addScheduleItem(item);

    if (gi)
        item_map[gi] = item;
}

void WeekView::onDoubleClicked(QGraphicsItem *i)
{
    QMap<QGraphicsItem *, ScheduleItem *>::iterator itr = item_map.find(i);
    if (itr != item_map.end())
        Q_EMIT editItem(itr.value());
}

void WeekView::showContextMenu(const QPoint &pos)
{
    menu->popup(viewport()->mapToGlobal(pos));
}

void WeekView::itemChanged(ScheduleItem *item)
{
    QMap<QGraphicsItem *, ScheduleItem *>::iterator i = item_map.begin();
    while (i != item_map.end()) {
        if (item == i.value()) {
            QGraphicsItem *gi = i.key();
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

#include "moc_weekview.cpp"
