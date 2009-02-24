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
#include <KConfigGroup>
#include "centralwidget.h"
#include "activitybar.h"

namespace kt
{
	CentralWidget::CentralWidget(QWidget* parent) : QWidget(parent),pos(LEFT)
	{
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
		layout->setMargin(0);
		layout->setSpacing(0);
		stack = new QStackedWidget(this);
		activity_bar = new ActivityBar(stack,this);
		layout->addWidget(activity_bar);
		layout->addWidget(stack);
		connect(activity_bar,SIGNAL(changePosition(ActivityListPosition)),this,SLOT(setActivityBarPosition(ActivityListPosition)));
	}

	CentralWidget::~CentralWidget() 
	{
	}

	void CentralWidget::loadState(KSharedConfigPtr cfg)
	{
		activity_bar->loadState(cfg);
		KConfigGroup g = cfg->group("MainWindow");
		int idx = g.readEntry("current_activity",0);
		activity_bar->setCurrentActivity((Activity*)stack->widget(idx));
		setActivityBarPosition((ActivityListPosition)g.readEntry("activity_bar_pos",(int)LEFT));
		activity_bar->setPosition(pos);
	}
	
	void CentralWidget::saveState(KSharedConfigPtr cfg)
	{
		activity_bar->saveState(cfg);
		KConfigGroup g = cfg->group("MainWindow");
		g.writeEntry("current_activity",stack->currentIndex());
		g.writeEntry("activity_bar_pos",(int)pos);
	}
	
	void CentralWidget::setActivityBarPosition(kt::ActivityListPosition p) 
	{
		if (p == pos)
			return;
		
		switch (p)
		{
			case LEFT:
				layout->setDirection(QBoxLayout::LeftToRight);
				activity_bar->setFlow(QListView::TopToBottom);
				break;
			case RIGHT:
				layout->setDirection(QBoxLayout::RightToLeft);
				activity_bar->setFlow(QListView::TopToBottom);
				break;
			case TOP:
				layout->setDirection(QBoxLayout::TopToBottom);
				activity_bar->setFlow(QListView::LeftToRight);
				break;
			case BOTTOM:
				layout->setDirection(QBoxLayout::BottomToTop);
				activity_bar->setFlow(QListView::LeftToRight);
				break;
		}
		
		pos = p;
	}

}

