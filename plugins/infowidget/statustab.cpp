/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#include <math.h>
#include <float.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <kglobal.h>
#include <klocale.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>

#include "downloadedchunkbar.h"
#include "availabilitychunkbar.h"
#include "floatspinbox.h"
#include "statustab.h"
		
namespace kt
{

	StatusTab::StatusTab(QWidget* parent, const char* name, WFlags fl)
			: StatusTabBase(parent,name,fl),curr_tc(0)
	{
		QColorGroup cg = colorGroup();
		// do not use hardcoded colors
		m_info_caption->setPaletteBackgroundColor(cg.mid());
		m_chunks_caption->setPaletteBackgroundColor(cg.mid());
		m_sharing_caption->setPaletteBackgroundColor(cg.mid()); 
		
		maxRatio->setMinValue(0.0f);
		maxRatio->setMaxValue(100.0f);
		maxRatio->setStep(0.1f);
		connect(maxRatio, SIGNAL(valueHasChanged()), this, SLOT(maxRatioReturnPressed()));
		connect(useLimit, SIGNAL( toggled(bool) ), this, SLOT( useLimitToggled(bool) ) );
		
		maxTime->setMinValue(0.0f);
		maxTime->setMaxValue(10000000.0f);
		maxTime->setStep(0.05f);
		maxTime->setSpecialValueText(i18n("No limit"));
		connect(useTimeLimit,SIGNAL(toggled(bool)), this,SLOT(useTimeLimitToggled(bool)));
		connect(maxTime,SIGNAL(valueHasChanged()), this, SLOT(timeValueChanged()));
		
		int h = (int)ceil(fontMetrics().height()*1.25);
		m_chunk_bar->setFixedHeight(h);
		m_av_chunk_bar->setFixedHeight(h);
	}
	
	StatusTab::~StatusTab()
	{}

	void StatusTab::changeTC(kt::TorrentInterface* tc)
	{
		if (tc == curr_tc)
			return;
	
		curr_tc = tc;
	
		m_chunk_bar->setTC(tc);
		m_av_chunk_bar->setTC(tc);
		setEnabled(tc != 0);
		
		if (curr_tc)
		{
			float ratio = curr_tc->getMaxShareRatio();
			if(ratio > 0)
			{
				useLimit->setChecked(true);
				maxRatio->setValue(ratio);
			}
			else
			{
				maxRatio->setValue(0.0);
				useLimit->setChecked(false);
				maxRatio->setEnabled(false);
			}
			
			float hours = curr_tc->getMaxSeedTime();
			if (hours > 0)
			{
				maxTime->setEnabled(true);
				useTimeLimit->setChecked(true);
				maxTime->setValue(hours);
			}
			else
			{
				maxTime->setEnabled(false);
				useTimeLimit->setChecked(false);
			}
		}
		else
		{
			maxRatio->setValue(0.00f);
			m_share_ratio->clear();
			m_tracker_status->clear();
			m_seeders->clear();
			m_leechers->clear();
			m_tracker_update_time->clear();
			m_avg_up->clear();
			m_avg_down->clear();
		}
		
		update();
	}
	
	void StatusTab::update()
	{
		if (!curr_tc)
			return;
	
		const TorrentStats & s = curr_tc->getStats();
		
		m_chunk_bar->updateBar();
		m_av_chunk_bar->updateBar();
		
		if (s.running)
		{
			QTime t;
			t = t.addSecs(curr_tc->getTimeToNextTrackerUpdate());
			m_tracker_update_time->setText(t.toString("mm:ss"));
		}
		else
		{
			m_tracker_update_time->setText("");
		}
		
		m_tracker_status->setText(s.trackerstatus);
		
		m_seeders->setText(QString("%1 (%2)")
				.arg(s.seeders_connected_to).arg(s.seeders_total));
	
		m_leechers->setText(QString("%1 (%2)")
				.arg(s.leechers_connected_to).arg(s.leechers_total));
	
		float ratio = kt::ShareRatio(s);
		if(!maxRatio->hasFocus() && useLimit->isChecked())
			maxRatioUpdate();
		
		m_share_ratio->setText(QString("<font color=\"%1\">%2</font>").arg(ratio <= 0.8 ? "#ff0000" : "#1c9a1c").arg(KGlobal::locale()->formatNumber(ratio,2)));
	
		Uint32 secs = curr_tc->getRunningTimeUL(); 
		if (secs == 0)
		{
			m_avg_up->setText(KBytesPerSecToString(0));
			
		}
		else
		{
			double r = (double)s.bytes_uploaded / 1024.0;
			m_avg_up->setText(KBytesPerSecToString(r / secs));
		}
		
		secs = curr_tc->getRunningTimeDL();
		if (secs == 0)
		{
			m_avg_down->setText(KBytesPerSecToString(0));
		}
		else
		{
			double r = (double)(s.bytes_downloaded - s.imported_bytes)/ 1024.0;
			m_avg_down->setText(KBytesPerSecToString(r / secs));
		}
	}
	
	void StatusTab::maxRatioReturnPressed()
	{
		if(!curr_tc)
			return;
		
		curr_tc->setMaxShareRatio(maxRatio->value());
	}
	
	void StatusTab::useLimitToggled(bool state)
	{
		if(!curr_tc)
			return;
		
		maxRatio->setEnabled(state);
		if (!state)
		{
			curr_tc->setMaxShareRatio(0.00f);
			maxRatio->setValue(0.00f);
		}
		else
		{
			float msr = curr_tc->getMaxShareRatio();
			if(msr == 0.00f)
			{	
				curr_tc->setMaxShareRatio(1.00f);
				maxRatio->setValue(1.00f);
			}
			
			float sr = kt::ShareRatio(curr_tc->getStats());
			if(sr >= 1.00f)
			{
				//always add 1 to max share ratio to prevent stopping if torrent is running.
				curr_tc->setMaxShareRatio(sr + 1.00f);
				maxRatio->setValue(sr + 1.00f);
			}
		}
	}
	
	void StatusTab::maxRatioUpdate()
	{
		if(!curr_tc)
			return;
		
		float ratio = curr_tc->getMaxShareRatio();
		if(ratio > 0.00f)
		{
			maxRatio->setEnabled(true);
			useLimit->setChecked(true);
			maxRatio->setValue(ratio);
		}
		else
		{
			maxRatio->setEnabled(false);
			useLimit->setChecked(false);
			maxRatio->setValue(0.00f);
		}
	}
	
	void StatusTab::useTimeLimitToggled(bool on)
	{
		if(!curr_tc)
			return;
		
		maxTime->setEnabled(on);
		if (on)
		{
			Uint32 dl = curr_tc->getRunningTimeDL();
			Uint32 ul = curr_tc->getRunningTimeUL();
			float hours = (ul - dl) / 3600.0f + 1.0; // add one hour to current seed time to not stop immediatly
			maxTime->setValue(hours); 
			curr_tc->setMaxSeedTime(hours);
		}
		else
		{
			curr_tc->setMaxSeedTime(0.0f);
		}
	}
	
	void StatusTab::timeValueChanged()
	{
		if (curr_tc)
			curr_tc->setMaxSeedTime(maxTime->value());
	}

}

#include "statustab.moc"

