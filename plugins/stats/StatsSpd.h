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

#ifndef STATSSPD_H_
#define STATSSPD_H_

#include <qwidget.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qgroupbox.h>

#include "statspluginsettings.h"
#include "statsspdwgt.h"
#include "ChartDrawer.h"

namespace kt {

/**
\brief Main widget of stats plugin
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class StatsSpd : public StatsSpdWgt
{	
	Q_OBJECT
	
	private:
		///Layout of upload speed
		QVBoxLayout * pmUSpdLay;
		///Layout of down speed
		QVBoxLayout * pmDSpdLay;
		///Layout of peers speed
		QVBoxLayout * pmPeersSpdLay;
		
		///Chart widget of download speed
		ChartDrawer * pmDownCht;
		///Chart widget of peers speed
		ChartDrawer * pmPeersSpdCht;
		///Chart widget of upload speed
		ChartDrawer * pmUpCht;

	public:
		/**
		\brief Constructor
		\param p Parent
		*/
		StatsSpd(QWidget *p = 0);
		///Destructor
		virtual ~StatsSpd();
		
		/**
		\brief Adds value to upload speed chart
		\param idx Dataset index
		\param val Value
		**/
		void AddUpSpdVal(const size_t idx, const double val);
		/**
		\brief Adds value to download speed chart
		\param idx Dataset index
		\param val Value
		**/
		void AddDownSpdVal(const size_t idx, const double val);
		/**
		\brief Adds value to peers speed chart
		\param idx Dataset index
		\param val Value
		**/
		void AddPeersSpdVal(const size_t idx, const double val);
		/**
		\brief Changes download chart's measurments count
		\param cnt Measurements
		*/
		void ChangeDownMsmtCnt(const size_t cnt);
		/**
		\brief Changes peers speed chart's measurments count
		\param cnt Measurements
		*/
		void ChangePrsSpdMsmtCnt(const size_t cnt);
		/**
		\brief Changes upload chart's measurments count
		\param cnt Measurements
		*/
		void ChangeUpMsmtCnt(const size_t cnt);
		/**
		\brief Changes charts OY axis maximum mode
		\param mm Mode
		*/
		void ChangeChartsMaxMode(const ChartDrawer::MaxMode mm);
	
	public slots:
		///Updates charts
		void UpdateCharts();
};

}

#endif
