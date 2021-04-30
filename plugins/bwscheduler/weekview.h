/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTWEEKVIEW_H
#define KTWEEKVIEW_H

#include <QGraphicsView>
#include <QMap>
#include <QMenu>

#include <util/constants.h>

namespace kt
{
struct ScheduleItem;
class Schedule;
class WeekScene;

/**
    Displays the schedule of one week.
*/
class WeekView : public QGraphicsView
{
    Q_OBJECT
public:
    WeekView(QWidget *parent);
    ~WeekView() override;

    /**
     * Set the current Schedule
     * @param s The current schedule
     */
    void setSchedule(Schedule *s);

    /**
     * Clear the current Schedule.
     */
    void clear();

    /// Get the selected items
    QList<ScheduleItem *> selectedItems()
    {
        return selection;
    }

    /**
     * Add an item to the schedule.
     * @param item The item to add
     */
    void addScheduleItem(ScheduleItem *item);

    /**
     * Remove all selected items from the schedule.
     */
    void removeSelectedItems();

    /// Get the right click menu
    QMenu *rightClickMenu()
    {
        return menu;
    }

    /**
     * Update the text of the status line
     * @param up Up speed
     * @param down Down speed
     * @param suspended Suspended or not
     * @param enabled Enabled or not
     */
    void updateStatusText(int up, int down, bool suspended, bool enabled);

    /**
     * Something has changed about an item
     * @param item
     */
    void itemChanged(ScheduleItem *item);

    /**
     * The color settings have changed.
     */
    void colorsChanged();

Q_SIGNALS:
    void selectionChanged();
    void editItem(ScheduleItem *item);
    void itemMoved(ScheduleItem *item, const QTime &start, const QTime &end, int start_day, int end_day);

private Q_SLOTS:
    void onSelectionChanged();
    void showContextMenu(const QPoint &pos);
    void onDoubleClicked(QGraphicsItem *i);

private:
    WeekScene *scene;

    Schedule *schedule;
    QMap<QGraphicsItem *, ScheduleItem *> item_map;
    QList<ScheduleItem *> selection;
    QMenu *menu;
};

}

#endif
