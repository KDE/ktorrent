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

#ifndef STATSCON_H_
#define STATSCON_H_

#include <qwidget.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qgroupbox.h>

#include "statspluginsettings.h"
#include "statsconwgt.h"
#include "ChartDrawer.h"

namespace kt {

class StatsCon : public StatsConWgt
{
	Q_OBJECT
	private:
		///Layout of peers connections
		QVBoxLayout * pmPeersConLay;
		///Layout of DHT stats
		QVBoxLayout * pmDHTLay;
		
		///Chart widget of peers connted
		ChartDrawer * pmPeersConCht;
		///Chart widget of DHT
		ChartDrawer * pmDHTCht;
	
	public:
		StatsCon(QWidget * p = 0);
		virtual ~StatsCon();
		/**
		\brief Adds value to peers connections chart
		\param idx Dataset index
		\param val Value
		*/
		void AddPeersConVal(const size_t idx, const double val);
		/**
		\brief Adds value to DHT chart
		\param idx Dataset index
		\param val Value
		*/
		void AddDHTVal(const size_t idx, const double val);
		
		/**
		\brief Zeroes data on given idx @ peers connections chart
		\param idx Dataset index
		*/
		void ZeroPeersConn(const size_t idx);
		
		/**
		\brief Changes connections chart's measurments count
		\param cnt Measurements
		*/
		void ChangeConnMsmtCnt(const size_t cnt);
		/**
		\brief Changes DHT chart's measurments count
		\param cnt Measurements
		*/
		void ChangeDHTMsmtCnt(const size_t cnt);
		/**
		\brief Changes charts OY axis maximum mode
		\param mm Mode
		*/
		void ChangeChartsMaxMode(const ChartDrawer::MaxMode mm);
	public slots:
		///Updates charts
		void UpdateCharts();
	
};

} // NS

#endif
