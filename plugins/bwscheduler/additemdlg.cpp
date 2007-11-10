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
#include <kcalendarsystem.h>
#include "additemdlg.h"
#include "schedule.h"

namespace kt
{

	AddItemDlg::AddItemDlg(Mode mode,QWidget* parent) : QDialog(parent)
	{
		setupUi(this);
		connect(m_paused,SIGNAL(toggled(bool)),m_upload_limit,SLOT(setDisabled(bool)));
		connect(m_paused,SIGNAL(toggled(bool)),m_download_limit,SLOT(setDisabled(bool)));
		
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		for (int i = 1;i <= 7;i++)
			m_day->addItem(cal->weekDayName(i));
		
		connect(m_from,SIGNAL(timeChanged(const QTime & )),this,SLOT(fromChanged(const QTime&)));
		connect(m_to,SIGNAL(timeChanged(const QTime & )),this,SLOT(toChanged(const QTime&)));
		
		if (mode == EDIT_ITEM)
			setWindowTitle(i18n("Edit an item"));
	}


	AddItemDlg::~AddItemDlg()
	{}

	void AddItemDlg::fromChanged(const QTime & time)
	{
		// ensure that from is allways smaller then to
		m_to->setMinimumTime(time.addSecs(60));
	}
	
	void AddItemDlg::toChanged(const QTime & time)
	{
		// ensure that from is allways smaller then to
		m_from->setMaximumTime(time.addSecs(-60));
	}

	bool AddItemDlg::execute(ScheduleItem* item)
	{
		m_from->setTime(item->start);
		m_to->setTime(item->end);
		m_day->setCurrentIndex(item->day - 1);
		m_paused->setChecked(item->paused);
		m_upload_limit->setValue(item->upload_limit);
		m_download_limit->setValue(item->download_limit);
		if (exec() == QDialog::Accepted)
		{
			item->start = m_from->time();
			item->end = m_to->time();
			item->day = m_day->currentIndex() + 1;
			item->upload_limit = m_upload_limit->value();
			item->download_limit = m_download_limit->value();
			item->paused = m_paused->isChecked();
			return true;
		}
		return false;
	}
}

#include "additemdlg.moc"
