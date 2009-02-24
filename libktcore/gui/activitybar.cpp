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
	
	
	ActivityBar::ActivityBar(QStackedWidget* stack,QWidget* parent) : QWidget(parent),stack(stack)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		alw = new ActivityListWidget(this);
		layout->addWidget(alw);
		connect(alw,SIGNAL(currentActivityChanged(Activity*)),this,SLOT(currentChanged(Activity*)));
	}

	ActivityBar::~ActivityBar()
	{
	}
	
	void ActivityBar::addActivity(Activity* act)
	{
		stack->addWidget(act);
		alw->addActivity(act);
	}
	
	void ActivityBar::removeActivity(Activity* act)
	{
		int idx = stack->indexOf(act);
		if (idx >= 0)
		{
			stack->removeWidget(act);
			alw->removeActivity(act);
		}
	}
	
	void ActivityBar::setCurrentActivity(Activity* act)
	{
		stack->setCurrentWidget(act);
		alw->setCurrentActivity(act);
	}
	
	Activity* ActivityBar::currentActivity()
	{
		return (Activity*)stack->currentWidget();
	}
	
	void ActivityBar::currentChanged(Activity* act)
	{
		stack->setCurrentWidget(act);
	}
	
	void ActivityBar::loadState(KSharedConfigPtr cfg)
	{
		alw->loadState(cfg);
	}
	
	void ActivityBar::saveState(KSharedConfigPtr cfg)
	{
		alw->saveState(cfg);
	}
}
