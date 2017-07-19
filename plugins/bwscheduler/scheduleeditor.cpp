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

#include "scheduleeditor.h"

#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KActionCollection>
#include <KLocalizedString>

#include <util/error.h>
#include "weekview.h"
#include "schedule.h"
#include "edititemdlg.h"


namespace kt
{


    ScheduleEditor::ScheduleEditor(QWidget* parent)
        : Activity(i18n("Bandwidth Schedule"), QStringLiteral("kt-bandwidth-scheduler"), 20, parent), schedule(nullptr)
    {
        setXMLGUIFile(QStringLiteral("ktorrent_bwschedulerui.rc"));
        setToolTip(i18n("Edit the bandwidth schedule"));
        QVBoxLayout* layout = new QVBoxLayout(this);
        view = new WeekView(this);
        layout->addWidget(view);
        layout->setMargin(0);
        layout->setSpacing(0);

        setupActions();

        clear_action->setEnabled(false);
        edit_item_action->setEnabled(false);
        remove_item_action->setEnabled(false);

        QMenu* menu = view->rightClickMenu();
        menu->addAction(new_item_action);
        menu->addAction(edit_item_action);
        menu->addAction(remove_item_action);
        menu->addSeparator();
        menu->addAction(clear_action);

        connect(view, &WeekView::selectionChanged, this, &ScheduleEditor::onSelectionChanged);
        connect(view, &WeekView::editItem, this, static_cast<void (ScheduleEditor::*)(ScheduleItem*)>(&ScheduleEditor::editItem));
        connect(view, &WeekView::itemMoved, this, &ScheduleEditor::itemMoved);
    }


    ScheduleEditor::~ScheduleEditor()
    {}

    QAction* ScheduleEditor::addAction(const QString& icon, const QString& text, const QString& name,
                                       QObject* obj, const char* slot)
    {
        KActionCollection* ac = part()->actionCollection();
        QAction * a = new QAction(QIcon::fromTheme(icon), text, this);
        connect(a, SIGNAL(triggered(bool)), obj, slot);
        ac->addAction(name, a);
        return a;
    }


    void ScheduleEditor::setupActions()
    {
        load_action = addAction(QStringLiteral("document-open"), i18n("Load Schedule"), QStringLiteral("schedule_load"), this, SLOT(load()));
        save_action = addAction(QStringLiteral("document-save"), i18n("Save Schedule"), QStringLiteral("schedule_save"), this, SLOT(save()));
        new_item_action = addAction(QStringLiteral("list-add"), i18n("New Item"), QStringLiteral("new_schedule_item"), this, SLOT(addItem()));
        remove_item_action = addAction(QStringLiteral("list-remove"), i18n("Remove Item"), QStringLiteral("remove_schedule_item"), this, SLOT(removeItem()));
        edit_item_action = addAction(QStringLiteral("edit-select-all"), i18n("Edit Item"), QStringLiteral("edit_schedule_item"), this, SLOT(editItem()));
        clear_action = addAction(QStringLiteral("edit-clear"), i18n("Clear Schedule"), QStringLiteral("schedule_clear"), this, SLOT(clear()));

        QWidgetAction* act = new QWidgetAction(this);
        enable_schedule = new QCheckBox(i18n("Scheduler Active"), this);
        enable_schedule->setToolTip(i18n("Activate or deactivate the scheduler"));
        act->setDefaultWidget(enable_schedule);
        part()->actionCollection()->addAction(QStringLiteral("schedule_active"), act);
        connect(enable_schedule, &QCheckBox::toggled, this, &ScheduleEditor::enableChecked);
    }


    void ScheduleEditor::setSchedule(Schedule* s)
    {
        schedule = s;
        view->setSchedule(s);
        onSelectionChanged();
        enable_schedule->setChecked(s->isEnabled());
        clear_action->setEnabled(s->count() > 0);
    }

    void ScheduleEditor::clear()
    {
        view->clear();
        schedule->clear();
        view->setSchedule(schedule);
        clear_action->setEnabled(false);
        edit_item_action->setEnabled(false);
        remove_item_action->setEnabled(false);
        scheduleChanged();
    }

    void ScheduleEditor::save()
    {
        QString fn = QFileDialog::getSaveFileName(this, QString(), i18n("KTorrent scheduler files") + QLatin1String(" (*.sched)"));
        if (!fn.isEmpty())
        {
            try
            {
                schedule->save(fn);
            }
            catch (bt::Error& err)
            {
                QMessageBox::critical(this, QString(), err.toString());
            }
        }
    }

    void ScheduleEditor::load()
    {
        QString fn = QFileDialog::getOpenFileName(this, QString(), i18n("KTorrent scheduler files") + QLatin1String(" (*.sched)"));
        if (!fn.isEmpty())
        {
            Schedule* s = new Schedule();
            try
            {
                s->load(fn);
                loaded(s);
            }
            catch (bt::Error& err)
            {
                QMessageBox::critical(this, QString(), err.toString());
                delete s;
            }
        }
    }

    void ScheduleEditor::addItem()
    {
        ScheduleItem* item = new ScheduleItem();
        item->start_day = 1;
        item->end_day = 7;
        item->start = QTime(10, 0);
        item->end = QTime(12, 0);
        item->checkTimes();
        EditItemDlg dlg(schedule, item, true, this);
        if (dlg.exec() == QDialog::Accepted && schedule->addItem(item))
        {
            clear_action->setEnabled(true);
            view->addScheduleItem(item);
            scheduleChanged();
        }
        else
            delete item;
    }

    void ScheduleEditor::removeItem()
    {
        view->removeSelectedItems();
        clear_action->setEnabled(schedule->count() > 0);
        scheduleChanged();
    }

    void ScheduleEditor::editItem(ScheduleItem* item)
    {
        ScheduleItem tmp = *item;

        EditItemDlg dlg(schedule, item, false, this);
        if (dlg.exec() == QDialog::Accepted)
        {
            if (schedule->conflicts(item))
            {
                *item = tmp; // restore old values
                QMessageBox::critical(this, QString(), i18n("This item conflicts with another item in the schedule, we cannot change it."));
            }
            else
            {
                view->itemChanged(item);
            }
            clear_action->setEnabled(schedule->count() > 0);
            scheduleChanged();
        }
    }

    void ScheduleEditor::editItem()
    {
        editItem(view->selectedItems().front());
    }

    void ScheduleEditor::onSelectionChanged()
    {
        bool on = view->selectedItems().count() > 0;
        edit_item_action->setEnabled(on);
        remove_item_action->setEnabled(on);
    }

    void ScheduleEditor::updateStatusText(int up, int down, bool suspended, bool enabled)
    {
        view->updateStatusText(up, down, suspended, enabled);
    }

    void ScheduleEditor::itemMoved(kt::ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day)
    {
        schedule->modify(item, start, end, start_day, end_day);
        view->itemChanged(item);
        scheduleChanged();
    }

    void ScheduleEditor::colorsChanged()
    {
        view->colorsChanged();
    }

    void ScheduleEditor::enableChecked(bool on)
    {
        schedule->setEnabled(on);
        scheduleChanged();
    }

}

