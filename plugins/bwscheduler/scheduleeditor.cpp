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
#include <QAction>
#include <QVBoxLayout>
#include <ktoolbar.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <util/error.h>
#include "scheduleeditor.h"
#include "weekview.h"
#include "schedule.h"
#include "additemdlg.h"
#include "edititemdlg.h"

namespace kt
{
	

	ScheduleEditor::ScheduleEditor(QWidget* parent) 
		: Activity(i18n("Bandwidth\nSchedule"),"kt-bandwidth-scheduler",20,parent),schedule(0)
	{
		setToolTip(i18n("Edit the bandwidth schedule"));
		QVBoxLayout* layout = new QVBoxLayout(this);
		view = new WeekView(this);
		tool_bar = new KToolBar(this);

		layout->addWidget(tool_bar);
		layout->addWidget(view);
		
		load_action = tool_bar->addAction(KIcon("document-open"),i18n("Load Schedule"),this,SLOT(load()));
		save_action = tool_bar->addAction(KIcon("document-save"),i18n("Save Schedule"),this,SLOT(save()));
		tool_bar->addSeparator();
		new_item_action = tool_bar->addAction(KIcon("list-add"),i18n("New Item"),this,SLOT(addItem()));
		remove_item_action = tool_bar->addAction(KIcon("list-remove"),i18n("Remove Item"),this,SLOT(removeItem()));
		edit_item_action = tool_bar->addAction(KIcon("edit-select-all"),i18n("Edit Item"),this,SLOT(editItem()));
		tool_bar->addSeparator();
		clear_action = tool_bar->addAction(KIcon("edit-clear"),i18n("Clear Schedule"),this,SLOT(clear()));
		enable_schedule = new QCheckBox(i18n("Scheduler Active"),tool_bar);
		enable_schedule->setToolTip(i18n("Activate or deactivate the scheduler"));
		tool_bar->addWidget(enable_schedule);
		connect(enable_schedule,SIGNAL(toggled(bool)),this,SLOT(enableChecked(bool)));
		
		clear_action->setEnabled(false);
		edit_item_action->setEnabled(false);
		remove_item_action->setEnabled(false);
		
		KMenu* menu = view->rightClickMenu();
		menu->addAction(new_item_action);
		menu->addAction(edit_item_action);
		menu->addAction(remove_item_action);
		menu->addSeparator();
		menu->addAction(clear_action);
		
		connect(view,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));
		connect(view,SIGNAL(editItem(ScheduleItem*)),this,SLOT(editItem(ScheduleItem*)));
		connect(view,SIGNAL(itemMoved(ScheduleItem*, const QTime&, const QTime&,int)),
				this,SLOT(itemMoved(ScheduleItem*, const QTime&, const QTime&,int)));
	}

	
	ScheduleEditor::~ScheduleEditor()
	{}

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
		QString fn = KFileDialog::getSaveFileName(KUrl(),"*.sched | " + i18n("KTorrent scheduler files"),this);
		if (!fn.isNull())
		{
			try 
			{
				schedule->save(fn);
			}
			catch (bt::Error & err)
			{
				KMessageBox::error(this,err.toString());
			}
		}
	}
	
	void ScheduleEditor::load()
	{
		QString fn = KFileDialog::getOpenFileName(KUrl(),"*.sched | " + i18n("KTorrent scheduler files") + "\n* |" + i18n("All files"),this);
		if (!fn.isNull())
		{
			Schedule* s = new Schedule();
			try 
			{
				s->load(fn);
				loaded(s);
			}
			catch (bt::Error & err)
			{
				KMessageBox::error(this,err.toString());
				delete s;
			}
		}
	}
	
	void ScheduleEditor::addItem()
	{
		AddItemDlg dlg(schedule,this);
		if (dlg.exec() == QDialog::Accepted)
		{
			clear_action->setEnabled(true);
			QList<ScheduleItem*> added_items = dlg.getAddedItems();
			foreach (ScheduleItem* item,added_items)
				view->addScheduleItem(item);
			scheduleChanged();
		}
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
		
		EditItemDlg dlg(this);
		if (dlg.execute(item))
		{
			if (schedule->conflicts(item))
			{
				*item = tmp; // restore old values
				KMessageBox::error(this,i18n("This item conflicts with another item in the schedule, we cannot change it."));
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
	
	void ScheduleEditor::updateStatusText(int up,int down,bool paused,bool enabled)
	{
		view->updateStatusText(up,down,paused,enabled);
	}
	
	void ScheduleEditor::itemMoved(ScheduleItem* item,const QTime & start,const QTime & end,int day)
	{
		schedule->modify(item,start,end,day);
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

#include "scheduleeditor.moc"
