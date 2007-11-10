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

namespace kt
{
	

	ScheduleEditor::ScheduleEditor(QWidget* parent) : QWidget(parent),schedule(0)
	{
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
		clear_action = tool_bar->addAction(KIcon("edit-delete"),i18n("Clear Schedule"),this,SLOT(clear()));
		
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
		connect(view,SIGNAL(editItem(const ScheduleItem&)),this,SLOT(editItem(const ScheduleItem&)));
	}


	ScheduleEditor::~ScheduleEditor()
	{}

	void ScheduleEditor::setSchedule(Schedule* s)
	{
		schedule = s;
		view->setSchedule(s);
		onSelectionChanged();
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
		ScheduleItem item;
		item.start = QTime::currentTime();
		item.end = item.start.addSecs(3600);
		item.day = 1;
		
		AddItemDlg dlg(AddItemDlg::NEW_ITEM,this);
		if (dlg.execute(&item))
		{
			if (!schedule->addItem(item))
				KMessageBox::error(this,i18n("This item conflicts with another item in the schedule, we cannot add it !"));
			else
				view->addScheduleItem(item); 
			
			clear_action->setEnabled(true);
		}
	}

	void ScheduleEditor::removeItem()
	{
		view->removeSelectedItems();
		clear_action->setEnabled(schedule->count() > 0);
	}
	
	void ScheduleEditor::editItem(const ScheduleItem & item)
	{
		ScheduleItem old_item = item;
		ScheduleItem new_item = old_item;
		
		AddItemDlg dlg(AddItemDlg::EDIT_ITEM,this);
		if (dlg.execute(&new_item))
		{
			schedule->removeAll(old_item);
			if (!schedule->addItem(new_item))
			{
				KMessageBox::error(this,i18n("This item conflicts with another item in the schedule, we cannot change it !"));
				schedule->addItem(old_item); // add the old one back again
			}
			else
			{
				view->clear();
				view->setSchedule(schedule);
			}
			clear_action->setEnabled(schedule->count() > 0);
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
}

#include "scheduleeditor.moc"
