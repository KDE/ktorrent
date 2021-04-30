/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCHEDULEEDITOR_H
#define KTSCHEDULEEDITOR_H

#include <interfaces/activity.h>

class QCheckBox;

namespace kt
{
class WeekView;
class Schedule;
struct ScheduleItem;

/**
    @author
*/
class ScheduleEditor : public Activity
{
    Q_OBJECT

    typedef void (ScheduleEditor::*Func)();

public:
    ScheduleEditor(QWidget *parent);
    ~ScheduleEditor() override;

    /**
     * Set the current Schedule
     * @param s The current schedule
     */
    void setSchedule(Schedule *s);

    /**
     * Update the text of the status line
     * @param up Up speed
     * @param down Down speed
     * @param suspended Suspended or not
     * @param enabled Enabled or not
     */
    void updateStatusText(int up, int down, bool suspended, bool enabled);

    /**
     * The color settings have changed
     */
    void colorsChanged();

private Q_SLOTS:
    void clear();
    void save();
    void load();
    void addItem();
    void removeItem();
    void editItem();
    void onSelectionChanged();
    void editItem(ScheduleItem *item);
    void itemMoved(ScheduleItem *item, const QTime &start, const QTime &end, int start_day, int end_day);
    void enableChecked(bool on);

Q_SIGNALS:
    /**
     * Emitted when the user loads a new schedule.
     * @param ns The new schedule
     */
    void loaded(Schedule *ns);

    /**
     * Emitted when something changes in the schedule.
     */
    void scheduleChanged();

private:
    void setupActions();
    QAction *addAction(const QString &icon, const QString &text, const QString &name, Func slot);

private:
    WeekView *view;
    Schedule *schedule;

    QAction *load_action;
    QAction *save_action;
    QAction *new_item_action;
    QAction *remove_item_action;
    QAction *edit_item_action;
    QAction *clear_action;
    QCheckBox *enable_schedule;
};

}

#endif
