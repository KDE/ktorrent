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
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcalendarsystem.h>
#include "additemdlg.h"
#include "schedule.h"
#include "weekdaymodel.h"

namespace kt
{
	
	

	AddItemDlg::AddItemDlg(Schedule* schedule,QWidget* parent) : KDialog(parent),schedule(schedule)
	{
		setupUi(mainWidget());
		connect(m_suspended,SIGNAL(toggled(bool)),this,SLOT(suspendedChanged(bool)));
		connect(m_screensaver_limits,SIGNAL(toggled(bool)),this,SLOT(screensaverLimitsToggled(bool)));
		m_ss_download_limit->setEnabled(false);
		m_ss_upload_limit->setEnabled(false);
		model = new WeekDayModel(this);
		m_day_list->setModel(model);
		
		connect(m_day_list,SIGNAL(clicked(QModelIndex)),this,SLOT(dayClicked(QModelIndex)));
		
		connect(m_from,SIGNAL(timeChanged(const QTime & )),this,SLOT(fromChanged(const QTime&)));
		connect(m_to,SIGNAL(timeChanged(const QTime & )),this,SLOT(toChanged(const QTime&)));
		connect(m_entire_week,SIGNAL(clicked()),this,SLOT(selectEntireWeek()));
		connect(m_weekdays_only,SIGNAL(clicked()),this,SLOT(selectWeekDays()));
		connect(m_weekend,SIGNAL(clicked()),this,SLOT(selectWeekend()));
		
		setWindowTitle(i18n("Add an item"));
		
		m_from->setTime(QTime(10,0,0));
		m_from->setMaximumTime(QTime(23,58,0));
		m_to->setTime(QTime(11,59,59));
		m_to->setMinimumTime(QTime(0,1,0));
	
		m_suspended->setChecked(false);
		m_upload_limit->setValue(0);
		m_download_limit->setValue(0);
		m_set_connection_limits->setChecked(false);
		m_max_conn_per_torrent->setEnabled(false);
		m_max_conn_per_torrent->setValue(0);
		m_max_conn_global->setValue(0);
		m_max_conn_global->setEnabled(false);
	}


	AddItemDlg::~AddItemDlg()
	{}
	
	void AddItemDlg::accept()
	{
		QList<int> cd = model->checkedDays();
		if (cd.count() == 0)
		{
			KMessageBox::error(this,i18n("No day has been selected."));
			return;
		}
		
		int failures = 0;
		foreach (int day,cd)
		{
			ScheduleItem* item = new ScheduleItem();
			item->day = day;
			item->start = m_from->time();
			item->end = m_to->time();
			item->upload_limit = m_upload_limit->value();
			item->download_limit = m_download_limit->value();
			item->suspended = m_suspended->isChecked();
			item->global_conn_limit = m_max_conn_global->value();
			item->torrent_conn_limit = m_max_conn_per_torrent->value();
			item->set_conn_limits = m_set_connection_limits->isChecked();
			item->screensaver_limits = m_screensaver_limits->isChecked();
			item->ss_download_limit = m_ss_download_limit->value();
			item->ss_upload_limit = m_ss_upload_limit->value();
			item->checkTimes();
			if (!schedule->addItem(item))
			{
				failures++;
				delete item;
			}
			else
				added_items << item;
		}
		
		if (failures == cd.count())
		{
			KMessageBox::error(this,i18n("Failed to add item, because it conflicts with another item on the schedule."));	
			QDialog::reject();
		}
		else if (failures > 0)
		{
			KMessageBox::sorry(this,i18n("This item could not be added to all selected days, because it conflicts with another item on the schedule."));
			QDialog::accept();
		}
		else
		{
			QDialog::accept();
		}
	}

	void AddItemDlg::fromChanged(const QTime & time)
	{
		// ensure that from is always smaller then to
		if (time >= m_to->time())
			m_to->setTime(time.addSecs(60));
	}
	
	void AddItemDlg::toChanged(const QTime & time)
	{
		// ensure that from is always smaller then to
		if (time <= m_from->time())
			m_from->setTime(time.addSecs(-60));
	}
	
	void AddItemDlg::selectEntireWeek()
	{
		for (int i = 0;i < 7;i++)
		{
			model->setData(model->index(i,0),Qt::Checked,Qt::CheckStateRole);
		}
	}
	
	void AddItemDlg::selectWeekDays()
	{
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		for (int i = 0;i < 7;i++)
		{
			int day = ((cal->weekStartDay() - 1) + i) % 7;
			if (day < 5)
				model->setData(model->index(day,0),Qt::Checked,Qt::CheckStateRole);
			else
				model->setData(model->index(day,0),Qt::Unchecked,Qt::CheckStateRole);
		}
	}
	
	void AddItemDlg::selectWeekend()
	{
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		for (int i = 0;i < 7;i++)
		{
			int day = ((cal->weekStartDay() - 1) + i) % 7;
			if (day < 5)
				model->setData(model->index(day,0),Qt::Unchecked,Qt::CheckStateRole);
			else
				model->setData(model->index(day,0),Qt::Checked,Qt::CheckStateRole);
		}
	}
	
	void AddItemDlg::suspendedChanged(bool on) 
	{
		m_upload_limit->setDisabled(on);
		m_download_limit->setDisabled(on);
		m_screensaver_limits->setDisabled(on);
		screensaverLimitsToggled(m_screensaver_limits->isChecked());
	}
	
	void AddItemDlg::screensaverLimitsToggled(bool on) 
	{
		m_ss_download_limit->setEnabled(!m_suspended->isChecked() && on);
		m_ss_upload_limit->setEnabled(!m_suspended->isChecked() && on);
	}
	
	void AddItemDlg::dayClicked(const QModelIndex& idx)
	{
		if (model->data(idx,Qt::CheckStateRole) != Qt::Checked)
			model->setData(idx,Qt::Checked,Qt::CheckStateRole);
		else
			model->setData(idx,Qt::Unchecked,Qt::CheckStateRole);
	}

}

#include "additemdlg.moc"
