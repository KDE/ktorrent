/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef ACTIVITYBAR_H
#define ACTIVITYBAR_H

#include <QList>
#include <QDockWidget>
#include <QStackedWidget>
#include "ktcore_export.h"

class QListWidgetItem;

namespace kt
{
	class Activity;
	class ActivityListWidget;
	
	/**
	 * Bar to switch between activities
	 */
	class KTCORE_EXPORT ActivityBar : public QDockWidget
	{
		Q_OBJECT
	public:
		ActivityBar(QStackedWidget* stack,QWidget* parent);
		virtual ~ActivityBar();
		
		void addActivity(Activity* act);
		void removeActivity(Activity* act);
		void setCurrentActivity(Activity* act);
		Activity* currentActivity();
		
	signals:
		void currentActivityChanged(Activity* act);
		
	private slots:
		void itemClicked(QListWidgetItem* it);
		
	private:
		QStackedWidget* stack;
		QList<Activity*> activities;
		ActivityListWidget* alw;
	};
}

#endif // ACTIVITYBAR_H
