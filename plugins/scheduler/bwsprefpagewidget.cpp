/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivan@ktorrent.org                                                     *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include "bwspage.h"
#include "bwsprefpagewidget.h"
#include "bwscheduler.h"
#include "schedulerpluginsettings.h"

#include <knuminput.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>

namespace kt
{

	/* PREF PAGE WIDGET --------------------------------------------------------*/

	BWSPrefPageWidget::BWSPrefPageWidget(QWidget* parent, const char* name, WFlags fl)
			: BWSPage(parent,name,fl)
	{
		bool useit = SchedulerPluginSettings::enableBWS();
		
		if(useit)
			loadDefault();
		
		useBS->setChecked(useit);

		lblStatus->clear();
		
		connect(useBS, SIGNAL(toggled(bool)), this, SLOT(use_toggled( bool )));
	}

	BWSPrefPageWidget::~BWSPrefPageWidget()
	{}

	void BWSPrefPageWidget::comboDay_activated(int i)
	{
		comboCategory->setCurrentItem( (int) schedule.getCategory(i, comboTime->currentItem()) );
		lblStatus->clear();
	}

	void BWSPrefPageWidget::comboTime_activated(int i)
	{
		comboCategory->setCurrentItem( (int) schedule.getCategory(comboDay->currentItem(), i) );
		lblStatus->clear();
	}

	void BWSPrefPageWidget::comboCategory_activated(int index)
	{
		schedule.setCategory(comboDay->currentItem(), comboTime->currentItem(), (ScheduleCategory) index);
		lblStatus->clear();
	}

	void BWSPrefPageWidget::btnSave_clicked()
	{
		QString sf = KFileDialog::getSaveFileName("/home","*",this,i18n("Choose a filename to save under"));

		if(sf.isEmpty())
			return;

		saveSchedule(sf);
	}

	void BWSPrefPageWidget::btnLoad_clicked()
	{
		QString lf = KFileDialog::getOpenFileName("/home", "*",this,i18n("Choose a file"));

		if(lf.isEmpty())
			return;
		
		btnReset_clicked();
		loadSchedule(lf);
	}
	
	void BWSPrefPageWidget::saveSchedule(QString& fn)
	{
		QFile file(fn);

		file.open(IO_WriteOnly);
		QDataStream stream(&file);

		//First category
		stream << dlCat1->value();
		stream << ulCat1->value();

		//Second category
		stream << dlCat2->value();
		stream << ulCat2->value();

		//Third category
		stream << dlCat3->value();
		stream << ulCat3->value();

		//Now schedule
		for(int i=0; i<7; ++i)
			for(int j=0; j<24; ++j)
				stream << (int) schedule.getCategory(i, j);

		file.close();
		lblStatus->setText(i18n("Schedule saved."));
	}

	void BWSPrefPageWidget::loadSchedule(QString& fn, bool showmsg)
	{
		QFile file(fn);

		if(!file.exists())
		{
			if(showmsg)
				KMessageBox::error(this, i18n("File not found."), i18n("Error"));
			return;
		}

		file.open(IO_ReadOnly);
		QDataStream stream(&file);

		int tmp;

		stream >> tmp;
		dlCat1->setValue(tmp);
		stream >> tmp;
		ulCat1->setValue(tmp);

		stream >> tmp;
		dlCat2->setValue(tmp);
		stream >> tmp;
		ulCat2->setValue(tmp);

		stream >> tmp;
		dlCat3->setValue(tmp);
		stream >> tmp;
		ulCat3->setValue(tmp);

		for(int i=0; i<7; ++i) {
			for(int j=0; j<24; ++j) {
				stream >> tmp;
				schedule.setCategory(i, j, (ScheduleCategory) tmp);
			}
		}

		file.close();
		comboDay->setCurrentItem(0);
		comboTime->setCurrentItem(0);
		comboCategory->setCurrentItem((int)schedule.getCategory(0, 0));

		lblStatus->setText(i18n("Schedule loaded."));
	}
	
	void BWSPrefPageWidget::loadDefault()
	{
		//read schedule from HD
		QString fn = KGlobal::dirs()->saveLocation("data","ktorrent") + "bwschedule";
		loadSchedule(fn, false);
	}
	
	void BWSPrefPageWidget::use_toggled(bool useit)
	{
		if(useit)
			loadDefault();
	}

	void BWSPrefPageWidget::btnReset_clicked()
	{
		schedule.reset();

		comboDay->setCurrentItem(0);
		comboTime->setCurrentItem(0);
		comboCategory->setCurrentItem(0);

		dlCat1->setValue(0);
		dlCat2->setValue(0);
		dlCat3->setValue(0);

		ulCat1->setValue(0);
		ulCat2->setValue(0);
		ulCat3->setValue(0);

		lblStatus->clear();
	}

	void BWSPrefPageWidget::apply()
	{
		SchedulerPluginSettings::setEnableBWS(useBS->isChecked());
		SchedulerPluginSettings::writeConfig();
		
		//update category values...
		schedule.setDownload(0, dlCat1->value());
		schedule.setUpload(0, ulCat1->value());
		schedule.setDownload(1, dlCat2->value());
		schedule.setUpload(1, ulCat2->value());
		schedule.setDownload(2, dlCat3->value());
		schedule.setUpload(2, ulCat3->value());
		
		
		if(useBS->isChecked())
		{
			//set new schedule
			BWScheduler::instance().setSchedule(schedule);
			
			/* force trigger since the schedule has changed but after KTorrent::apply()
			* Used QTimer with fixed interval - not very nice solution... */
			QTimer::singleShot(1000, this, SLOT(scheduler_trigger()));
		}
	}
	
	void BWSPrefPageWidget::scheduler_trigger()
	{
		BWScheduler::instance().trigger();
	}
}

#include "bwsprefpagewidget.moc"
