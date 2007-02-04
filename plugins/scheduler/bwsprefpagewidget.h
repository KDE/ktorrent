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
#ifndef KTBWSPREFPAGEWIDGET_H
#define KTBWSPREFPAGEWIDGET_H

#include <qwidget.h>

#include "bwspage.h"
#include "bwscheduler.h"

namespace kt
{
	/**
	 * @brief Bandwidth Scheduler page
	 * @author Ivan Vasic <ivasic@gmail.com>
	 */
	class BWSPrefPageWidget : public BWSPage
	{
			Q_OBJECT
		public:
			BWSPrefPageWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
			~BWSPrefPageWidget();
			/*$PUBLIC_FUNCTIONS$*/
			
			/**
			 * @brief Loads default schedule.
			 * Default schedule is currently active (if enabled) and it's in ~/.kde/share/apps/ktorrent/bwschedule
			 */
			void loadDefault();
			
			/**
			 * Loads a schedule from HD.
			 * @param fn Schedule filename
			 * @param showmsg Should I show msgBox if file doesn't exist.
			 * @ref BWSPrefPageWidget::btnLoad_clicked()
			 * @ref BWSPrefPageWidget::loadDefault()
			 */
			void loadSchedule(QString& fn, bool showmsg = true);
			
			/**
			 * Saves current schedule to HD.
			 * @param fn Schedule filename.
			 */
			void saveSchedule(QString& fn);
			

		public slots:
			/*$PUBLIC_SLOTS$*/
			virtual void btnReset_clicked();
			virtual void btnLoad_clicked();
			virtual void btnSave_clicked();
			virtual void btnApply_clicked();
			virtual void btnOk_clicked();
			
		private slots:
			void categoryChanged(int);

			///Applies settings
			void apply();

		private:
			BWS schedule;
	};

}

#endif
