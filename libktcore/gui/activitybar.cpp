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
#include <QVBoxLayout>
#include <klocale.h>
#include <util/log.h>
#include <interfaces/activity.h>
#include "activitybar.h"
#include "activitylistwidget.h"

using namespace bt;

namespace kt
{
	
	
	ActivityBar::ActivityBar(QStackedWidget* stack,QWidget* parent) : ActivityListWidget(parent),stack(stack)
	{
	}

	ActivityBar::~ActivityBar()
	{
	}
	
	void ActivityBar::addActivity(Activity* act)
	{
		stack->addWidget(act);
		ActivityListWidget::addActivity(act);
	}
	
	void ActivityBar::removeActivity(Activity* act)
	{
		int idx = stack->indexOf(act);
		if (idx >= 0)
		{
			stack->removeWidget(act);
			ActivityListWidget::removeActivity(act);
		}
	}
	
	void ActivityBar::setCurrentActivity(Activity* act)
	{
		stack->setCurrentWidget(act);
		ActivityListWidget::setCurrentActivity(act);
	}
	
	Activity* ActivityBar::currentActivity()
	{
		return (Activity*)stack->currentWidget();
	}
	
	void ActivityBar::currentActivityChanged(Activity* act)
	{
		stack->setCurrentWidget(act);
	}
}
