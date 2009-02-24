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
#include <kconfiggroup.h>
#include <kiconloader.h>
#include "tabbarwidget.h"

namespace kt
{
	TabBarWidget::TabBarWidget(QWidget* parent) 
		: QWidget(parent),widget_stack(0),next_id(1),shrunken(false)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		tab_bar = new KMultiTabBar(KMultiTabBar::Bottom,this);
		tab_bar->setStyle(KMultiTabBar::KDEV3ICON);
		widget_stack = new QStackedWidget(this);
		layout->addWidget(widget_stack);
		layout->addWidget(tab_bar);
		
		QSizePolicy tsp = sizePolicy();
		QSizePolicy wsp = widget_stack->sizePolicy();
		
		tsp.setVerticalPolicy(QSizePolicy::Fixed);
		wsp.setVerticalPolicy(QSizePolicy::Expanding);
		
		widget_stack->setSizePolicy(wsp);
		setSizePolicy(tsp);
		shrink();
	}
	
	TabBarWidget::~TabBarWidget()
	{
	}
	
	
	void TabBarWidget::addTab(QWidget* ti,const QString & text,const QString & icon,const QString & tooltip)
	{
		// get the next ID number
		int id = next_id++;
		
		Tab t = {ti,id,text,icon};
		tabs.append(t);
		// add the tab
		QPixmap pix = SmallIcon(icon);
		
		tab_bar->appendTab(pix,id,text);
		widget_stack->addWidget(ti);
		connect(tab_bar->tab(id),SIGNAL(clicked(int)),this,SLOT(onTabClicked(int)));
		
		tab_bar->setTab(id,false);
		show();
		tab_bar->tab(id)->setToolTip(tooltip);
	}
	
	
	void TabBarWidget::removeTab(QWidget* ti)
	{
		TabItr t = findByWidget(ti);
		if (t == tabs.end())
			return;
		
		int id = t->id;
		
		if (widget_stack->currentWidget() == ti)
		{
			tab_bar->removeTab(id);
			tabs.erase(t);
			ti->hide();
			widget_stack->removeWidget(ti);
			ti->setParent(0);
		}
		else
		{
			tab_bar->removeTab(id);
			tabs.erase(t);
			ti->setParent(0);
		}
		
		if (tabs.count() == 0)
		{
			widget_stack->hide();
			hide();
		}
	}
	
	void TabBarWidget::changeTabIcon(QWidget* ti,const QString & icon)
	{
		TabItr i = findByWidget(ti);
		if (i == tabs.end())
			return;
		
		tab_bar->tab(i->id)->setIcon(icon);
		i->icon = icon;
	}
	
	void TabBarWidget::changeTabText(QWidget* ti,const QString & text)
	{
		TabItr i = findByWidget(ti);
		if (i == tabs.end())
			return;
		
		tab_bar->tab(i->id)->setText(text);
		i->text = text;
	}
	
	void TabBarWidget::shrink()
	{
		widget_stack->hide();
		shrunken = true;
	}
	
	void TabBarWidget::unshrink()
	{
		widget_stack->show();
		shrunken = false;
	}
	
	void TabBarWidget::onTabClicked(int id)
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
				tab_bar->setTab(id,true);
				unshrink();
			}
			else
			{
				tab_bar->setTab(id,false);
				shrink();
			}
		}
		else
		{
			// update tab
			QWidget* current = widget_stack->currentWidget();
			
			tab_bar->setTab(findByWidget(current)->id,false);
			tab_bar->setTab(id,true);
			// change the current in stack
			widget_stack->setCurrentWidget(ti);
			if (shrunken)
				unshrink();
		}
	}
	
	
	void TabBarWidget::saveState(KSharedConfigPtr cfg,const QString & group)
	{
		QWidget* current = widget_stack->currentWidget();
		
		KConfigGroup g = cfg->group(group);
		g.writeEntry("shrunken",shrunken);
		if (current)
			g.writeEntry("current_tab",findByWidget(current)->text);
	}
	
	void TabBarWidget::loadState(KSharedConfigPtr cfg,const QString & group)
	{
		KConfigGroup g = cfg->group(group);
		
		bool tmp = g.readEntry("shrunken",true);
		if (tmp != shrunken)
		{
			if (tmp)
				shrink();
			else
				unshrink();
		}
		
		QString ctab = g.readPathEntry("current_tab", QString());
		TabItr i = findByText(ctab);
		if (i == tabs.end())
			return;
		
		bool shrunken_tmp = shrunken;
		onTabClicked(i->id);
		if (shrunken_tmp)
		{
			tab_bar->setTab(i->id,false);
			shrink();
		}
	}
	
	TabBarWidget::TabItr TabBarWidget::findByWidget(QWidget* w)
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
	
	TabBarWidget::TabItr TabBarWidget::findById(int id)
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
	
	TabBarWidget::TabItr TabBarWidget::findByText(const QString & text)
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