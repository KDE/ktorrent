/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include "schedulerprefpagewidget.h"
#include "bwsprefpagewidget.h"
#include "bwscheduler.h"
#include "schedulerpluginsettings.h"

#include <qcheckbox.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

namespace kt
{

	SchedulerPrefPageWidget::SchedulerPrefPageWidget(QWidget* parent, const char* name, WFlags fl)
			: SchedulerPage(parent,name,fl)
	{
		groupBWS->setEnabled(false);
		bool useit = SchedulerPluginSettings::enableBWS();
		bool use_colors = SchedulerPluginSettings::useColors();
		useBS->setChecked(useit);
		useColors->setChecked(use_colors);
	}


	SchedulerPrefPageWidget::~SchedulerPrefPageWidget()
	{}

	void SchedulerPrefPageWidget::btnEditBWS_clicked()
	{
		BWSPrefPageWidget w(this);
		w.exec();
	}
	
	void SchedulerPrefPageWidget::apply()
	{
		bool use_bws = useBS->isChecked();
		
		SchedulerPluginSettings::setEnableBWS(use_bws);
		SchedulerPluginSettings::setUseColors(useColors->isChecked());
		SchedulerPluginSettings::writeConfig();
		
		/* force trigger since the schedule has changed but after KTorrent::apply()
		* Used QTimer with fixed interval - not very nice solution... */
		if(useBS->isChecked())
			QTimer::singleShot(1000, this, SLOT(scheduler_trigger()));
		
		BWScheduler::instance().setEnabled(use_bws);
	}
	
	void SchedulerPrefPageWidget::scheduler_trigger()
	{
		BWScheduler::instance().trigger();
	}
	
	void SchedulerPrefPageWidget::useColors_toggled(bool)
	{
		SchedulerPluginSettings::setUseColors(useColors->isChecked());
		SchedulerPluginSettings::writeConfig();
	}

}



