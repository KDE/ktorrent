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
#ifndef KTSCHEDULERPREFPAGEWIDGET_H
#define KTSCHEDULERPREFPAGEWIDGET_H

#include <qwidget.h>
#include "schedulerpage.h"

namespace kt
{
	/**
	 * @brief Scheduler Preferences Page.
	 * @author Ivan Vasic <ivasic@gmail.com>
	 */
	class SchedulerPrefPageWidget : public SchedulerPage
	{
			Q_OBJECT
		public:
			SchedulerPrefPageWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

			~SchedulerPrefPageWidget();

			void apply();
			
		public slots:
    		virtual void btnEditBWS_clicked();
			void scheduler_trigger();
    		virtual void useColors_toggled(bool);
	};

}

#endif
