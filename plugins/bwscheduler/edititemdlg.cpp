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
#include "edititemdlg.h"
#include "schedule.h"

namespace kt
{

	EditItemDlg::EditItemDlg(QWidget* parent) : KDialog(parent)
	{
		setupUi(mainWidget());
		connect(m_paused,SIGNAL(toggled(bool)),this,SLOT(pausedChanged(bool)));
		connect(m_screensaver_limits,SIGNAL(toggled(bool)),this,SLOT(screensaverLimitsToggled(bool)));
		
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		for (int i = 1;i <= 7;i++)
			m_day->addItem(cal->weekDayName(i));
		
		connect(m_from,SIGNAL(timeChanged(const QTime & )),this,SLOT(fromChanged(const QTime&)));
		connect(m_to,SIGNAL(timeChanged(const QTime & )),this,SLOT(toChanged(const QTime&)));
		
	
		setWindowTitle(i18n("Edit an item"));
	}


	EditItemDlg::~EditItemDlg()
	{}

	void EditItemDlg::fromChanged(const QTime & time)
	{
		// ensure that from is always smaller then to
		m_to->setMinimumTime(time.addSecs(60));
	}
	
	void EditItemDlg::toChanged(const QTime & time)
	{
		// ensure that from is always smaller then to
		m_from->setMaximumTime(time.addSecs(-60));
	}
	
	void EditItemDlg::pausedChanged(bool on) 
	{
		m_upload_limit->setDisabled(on);
		m_download_limit->setDisabled(on);
		m_screensaver_limits->setDisabled(on);
		screensaverLimitsToggled(m_screensaver_limits->isChecked());
	}

	void EditItemDlg::screensaverLimitsToggled(bool on) 
	{
		m_ss_download_limit->setEnabled(!m_paused->isChecked() && on);
		m_ss_upload_limit->setEnabled(!m_paused->isChecked() && on);
	}


	bool EditItemDlg::execute(ScheduleItem* item)
	{
		m_from->setTime(item->start);
		m_to->setTime(item->end);
		m_day->setCurrentIndex(item->day - 1);
		m_paused->setChecked(item->paused);
		m_upload_limit->setValue(item->upload_limit);
		m_download_limit->setValue(item->download_limit);
		m_set_connection_limits->setChecked(item->set_conn_limits);
		m_max_conn_per_torrent->setEnabled(item->set_conn_limits);
		m_max_conn_per_torrent->setValue(item->torrent_conn_limit);
		m_max_conn_global->setValue(item->global_conn_limit);
		m_max_conn_global->setEnabled(item->set_conn_limits);
		m_screensaver_limits->setChecked(item->screensaver_limits);
		m_screensaver_limits->setEnabled(!item->paused);
		m_ss_download_limit->setValue(item->ss_download_limit);
		m_ss_upload_limit->setValue(item->ss_upload_limit);
		m_ss_download_limit->setEnabled(!item->paused && item->screensaver_limits);
		m_ss_upload_limit->setEnabled(!item->paused && item->screensaver_limits);
		if (exec() == QDialog::Accepted)
		{
			item->start = m_from->time();
			item->end = m_to->time(); 
			item->day = m_day->currentIndex() + 1;
			item->upload_limit = m_upload_limit->value();
			item->download_limit = m_download_limit->value();
			item->paused = m_paused->isChecked();
			item->global_conn_limit = m_max_conn_global->value();
			item->torrent_conn_limit = m_max_conn_per_torrent->value();
			item->set_conn_limits = m_set_connection_limits->isChecked();
			item->screensaver_limits = m_screensaver_limits->isChecked();
			item->ss_download_limit = m_ss_download_limit->value();
			item->ss_upload_limit = m_ss_upload_limit->value();
			item->checkTimes();
			return true;
		}
		return false;
	}
}

#include "edititemdlg.moc"
