/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "StatsSpd.h"

namespace kt {

StatsSpd::StatsSpd(QWidget *p) : StatsSpdWgt(p),
				pmDownCht(new ChartDrawer(DownSpeedGbw, StatsPluginSettings::downloadMeasurements())),
				pmPeersSpdCht(new ChartDrawer(PeersSpdGbw, StatsPluginSettings::peersSpeedMeasurements())),
				pmUpCht(new ChartDrawer(UpSpeedGbw, StatsPluginSettings::uploadMeasurements()))
{

	DownSpeedGbw->setColumnLayout(0, Qt::Vertical );
	DownSpeedGbw->layout()->setSpacing( 6 );
	DownSpeedGbw->layout()->setMargin( 11 );
	
	pmDSpdLay = new QVBoxLayout(DownSpeedGbw -> layout());
	
	UpSpeedGbw->setColumnLayout(0, Qt::Vertical );
	UpSpeedGbw->layout()->setSpacing( 6 );
	UpSpeedGbw->layout()->setMargin( 11 );
	
	pmUSpdLay = new QVBoxLayout(UpSpeedGbw -> layout());
	
	PeersSpdGbw->setColumnLayout(0, Qt::Vertical );
	PeersSpdGbw->layout()->setSpacing( 6 );
	PeersSpdGbw->layout()->setMargin( 11 );
	
	pmPeersSpdLay = new QVBoxLayout(PeersSpdGbw -> layout());

	//-----------------
	
	pmUSpdLay -> addWidget(pmUpCht);
	pmDSpdLay -> addWidget(pmDownCht);
	pmPeersSpdLay -> addWidget(pmPeersSpdCht);
	
	// ----------------
	
	pmUpCht -> AddValuesCnt(QPen("#f00"), i18n("Current"));
	pmDownCht -> AddValuesCnt(QPen("#f00"), i18n("Current"));

	pmUpCht -> AddValuesCnt(QPen("#00f"), i18n("Average"));
	pmDownCht -> AddValuesCnt(QPen("#00f"), i18n("Average"));
	
	pmUpCht -> AddValuesCnt(i18n("Limit"), 0);
	pmDownCht -> AddValuesCnt(i18n("Limit"), 0);
	
// 	pmUpCht -> AddValuesCnt(QPen("#f0f"), i18n("Current torrent"));
// 	pmDownCht -> AddValuesCnt(QPen("#f0f"), i18n("Current torrent"));
// 	
	
	
	pmPeersSpdCht -> AddValuesCnt(QPen("#090"), i18n("Average from leecher"));
	pmPeersSpdCht -> AddValuesCnt(QPen("#f00"), i18n("Average to leecher"));
	pmPeersSpdCht -> AddValuesCnt(QPen("#00f"), i18n("Average from seeder"));
	pmPeersSpdCht -> AddValuesCnt(QPen("magenta"), i18n("From leechers"));
	pmPeersSpdCht -> AddValuesCnt(QPen("orange"), i18n("From seeders"));
	
}

StatsSpd::~StatsSpd()
{
	delete pmUpCht;
	delete pmDownCht;
	delete pmPeersSpdCht;
		
	delete pmUSpdLay;
	delete pmDSpdLay;
	delete pmPeersSpdLay;

}

void StatsSpd::AddUpSpdVal(const size_t idx, const double val)
{
	pmUpCht -> AddValue(idx, val, false);
}

void StatsSpd::AddDownSpdVal(const size_t idx, const double val)
{
	pmDownCht -> AddValue(idx, val , false);
}

void StatsSpd::AddPeersSpdVal(const size_t idx, const double val)
{
	pmPeersSpdCht -> AddValue(idx, val , false);
}

void StatsSpd::UpdateCharts()
{
	pmUpCht -> update();
	pmDownCht -> update();
	pmPeersSpdCht -> update();
}

void StatsSpd::ChangeDownMsmtCnt(const size_t cnt)
{
	pmDownCht -> SetXMax(cnt);
}

void StatsSpd::ChangePrsSpdMsmtCnt(const size_t cnt)
{
	pmPeersSpdCht -> SetXMax(cnt);
}

void StatsSpd::ChangeUpMsmtCnt(const size_t cnt)
{
	pmUpCht -> SetXMax(cnt);
}

void StatsSpd::ChangeChartsMaxMode(const ChartDrawer::MaxMode mm)
{
	pmUpCht -> SetMaxMode(mm);
	pmDownCht -> SetMaxMode(mm);
	pmPeersSpdCht -> SetMaxMode(mm);
}


} //NS end

#include "StatsSpd.moc"
