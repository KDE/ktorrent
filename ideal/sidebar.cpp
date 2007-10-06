/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <stdio.h>
#include <QPixmap>
#include <QSplitter>
#include <QBoxLayout>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kconfiggroup.h>
#include "sidebar.h"
#include "mainwindow.h"
#include "box.h"

namespace ideal
{

	SideBar::SideBar(Box* parent,QSplitter* split,KMultiTabBarPosition pos) 
		: KMultiTabBar(pos,parent),
		split(split),widget_stack(0),next_id(1),shrunken(false)
	{
		setStyle(KMultiTabBar::KDEV3ICON);
		widget_stack = new QStackedWidget(split);
		
		if (pos == Left)
		{
			parent->insertWidget(this,0);
			split->insertWidget(0,widget_stack);
		}
		else
		{
			parent->addWidget(this);
			split->addWidget(widget_stack);
		}
		
		
		QSizePolicy tsp = sizePolicy();
		QSizePolicy wsp = widget_stack->sizePolicy();
		
		switch (pos) 
		{
		case Left:
		case Right:
			tsp.setHorizontalPolicy(QSizePolicy::Fixed);
			wsp.setHorizontalPolicy(QSizePolicy::Expanding);
			break;
		case Bottom:
		case Top:
			tsp.setVerticalPolicy(QSizePolicy::Fixed);
			wsp.setVerticalPolicy(QSizePolicy::Expanding);
			break;
		}
		
		widget_stack->setSizePolicy(wsp);
		setSizePolicy(tsp);
		shrink();
	}
	
	SideBar::~SideBar()
	{
	}
	

	void SideBar::addTab(QWidget* ti,const QString & text,const QString & icon)
	{
		// get the next ID number
		int id = next_id++;
	
		Tab t = {ti,id,text,icon};
		tabs.append(t);
		// add the tab
		QPixmap pix = SmallIcon(icon);
		
		this->appendTab(pix,id,text);
		widget_stack->addWidget(ti);
		connect(this->tab(id),SIGNAL(clicked(int)),this,SLOT(onTabClicked(int)));
		
		this->setTab(id,false);
		show();
	}

	
	void SideBar::removeTab(QWidget* ti)
	{
		TabItr t = findByWidget(ti);
		if (t == tabs.end())
			return;

		int id = t->id;
		
		if (widget_stack->currentWidget() == ti)
		{
			KMultiTabBar::removeTab(id);
			tabs.erase(t);
			ti->hide();
			widget_stack->removeWidget(ti);
			ti->setParent(0);
		}
		else
		{
			KMultiTabBar::removeTab(id);
			tabs.erase(t);
			ti->setParent(0);
		}

		if (tabs.count() == 0)
		{
			widget_stack->hide();
			hide();
		}
	}

	void SideBar::changeTabIcon(QWidget* ti,const QString & icon)
	{
		TabItr i = findByWidget(ti);
		if (i == tabs.end())
			return;

		this->tab(i->id)->setIcon(icon);
		i->icon = icon;
	}

	void SideBar::changeTabText(QWidget* ti,const QString & text)
	{
		TabItr i = findByWidget(ti);
		if (i == tabs.end())
			return;

		this->tab(i->id)->setText(text);
		i->text = text;
	}

	void SideBar::shrink()
	{
		widget_stack->hide();
		shrunken = true;
	}

	void SideBar::unshrink()
	{
		widget_stack->show();
		shrunken = false;
	}

	void SideBar::onTabClicked(int id)
	{
		TabItr i = findById(id);
		if (i == tabs.end())
			return;

		QWidget* ti = i->widget;
		if (ti == widget_stack->currentWidget())
		{
			// it is the current tab, so just toggle the visible property of the widget_stack
			if (!widget_stack->isVisible())
			{
				this->setTab(id,true);
				unshrink();
			}
			else
			{
				this->setTab(id,false);
				shrink();
			}
		}
		else
		{
			// update tab
			QWidget* current = widget_stack->currentWidget();

			this->setTab(findByWidget(current)->id,false);
			this->setTab(id,true);
			// change the current in stack
			widget_stack->setCurrentWidget(ti);
			if (shrunken)
				unshrink();
		}
	}

	QString SideBarGroupName(KMultiTabBar::KMultiTabBarPosition pos)
	{
		if (pos == SideBar::Left)
			return "LeftSideBar";
		else if (pos == SideBar::Right)
			return "RightSideBar";
		else
			return "BottomSideBar";
	}

	
	void SideBar::saveState(KSharedConfigPtr cfg)
	{
		QWidget* current = widget_stack->currentWidget();

		KConfigGroup g = cfg->group(SideBarGroupName(position()));
		g.writeEntry("shrunken",shrunken);
		if (current)
			g.writeEntry("current_tab",findByWidget(current)->text);
	}

	void SideBar::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group(SideBarGroupName(position()));

		bool tmp = g.readEntry("shrunken",true);
		if (tmp != shrunken)
		{
			if (tmp)
				shrink();
			else
				unshrink();
		}

		QString ctab = g.readPathEntry("current_tab");
		TabItr i = findByText(ctab);
		if (i == tabs.end())
			return;

		bool shrunken_tmp = shrunken;
		onTabClicked(i->id);
		if (shrunken_tmp)
		{
			setTab(i->id,false);
			shrink();
		}
	}

	SideBar::TabItr SideBar::findByWidget(QWidget* w)
	{
		TabItr i = tabs.begin();
		while (i != tabs.end())
		{
			if (i->widget == w)
				return i;
			i++;
		}
		return i;
	}

	SideBar::TabItr SideBar::findById(int id)
	{
		TabItr i = tabs.begin();
		while (i != tabs.end())
		{
			if (i->id == id)
				return i;
			i++;
		}
		return i;
	}

	SideBar::TabItr SideBar::findByText(const QString & text)
	{
		TabItr i = tabs.begin();
		while (i != tabs.end())
		{
			if (i->text == text)
				return i;
			i++;
		}
		return i;
	}

}

#include "sidebar.moc"

