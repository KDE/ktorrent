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
#include <QSplitter>
#include <ktabwidget.h>
#include <kicon.h>
#include <kmultitabbar.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kpushbutton.h>
#include "mainwindow.h"
#include "sidebar.h"
#include "box.h"


namespace ideal
{
	MainWindow::MainWindow()
	{
		tabs = new KTabWidget(this);
		left = right = bottom = 0;
		hsplit = vsplit = 0;
		left_corner = right_corner = 0;

		setupSplitters();
		connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(onCurrentTabChanged(int)));
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::loadState(KSharedConfigPtr cfg)
	{
		setAutoSaveSettings();
		if (left)
			left->loadState(cfg);
		if (right)
			right->loadState(cfg);
		if (bottom)
			bottom->loadState(cfg);
		loadSplitterState(cfg);
		
		KConfigGroup g = cfg->group("MainTabWidget");
		int ct = g.readEntry("current_tab",0);
		if (ct >= 0 && ct < tabs->count())
			tabs->setCurrentIndex(ct);
	}

	void MainWindow::saveState(KSharedConfigPtr cfg)
	{
		if (left)
			left->saveState(cfg);
		if (right)
			right->saveState(cfg);
		if (bottom)
			bottom->saveState(cfg);
		
		saveSplitterState(cfg);
		// save the current tab
		KConfigGroup g = cfg->group("MainTabWidget");
		g.writeEntry("current_tab",tabs->currentIndex());
		cfg->sync();
	}

	void MainWindow::setupSplitters()
	{
		// main widget is the vertical box
		vbox = new Box(true,this);
		setCentralWidget(vbox);

		// vertical box contains the vsplit, which has the hbox as child
		hbox = new Box(false,this);
		vsplit = new QSplitter(Qt::Vertical,this);
		vbox->addWidget(vsplit);
		vsplit->addWidget(hbox);

		// hsplit is part of hbox, tabs are part of hsplit
		hsplit = new QSplitter(Qt::Horizontal,this);
		hsplit->addWidget(tabs);
		hbox->addWidget(hsplit);
		
		vsplit->setChildrenCollapsible(false);
		hsplit->setChildrenCollapsible(false);
		tabs->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	}

	void MainWindow::createDockArea(TabPosition pos)
	{
		if (pos == LEFT && !left)
		{
			left = new SideBar(hbox,hsplit,SideBar::Left);
		}
		else if (pos == RIGHT && !right)
		{
			right = new SideBar(hbox,hsplit,SideBar::Right);
		}
		else if (pos == BOTTOM && !bottom)
		{
			bottom = new SideBar(vbox,vsplit,SideBar::Bottom);
		}
	}


	SideBar* MainWindow::getTabBar(TabPosition pos,bool create)
	{
		SideBar* t = 0;
		switch (pos)
		{
		case LEFT: 
			if (!left && create)
				createDockArea(LEFT);
			t = left; 
			break;
		case RIGHT: 
			if (!right && create)
				createDockArea(RIGHT);
			t = right; 
			break;
		case BOTTOM: 
			if (!bottom && create)
				createDockArea(BOTTOM);
			t = bottom; 
			break;
		default: 
			break;
		}
		return t;
	}

	void MainWindow::addTab(QWidget* widget,const QString & text,const QString & icon,TabPosition pos)
	{
		if (pos == CENTER)
		{
			KTabWidget* t = tabs;
			
			if (icon != QString::null)
				t->addTab(widget,KIcon(icon),text);
			else
				t->addTab(widget,text);
			t->setCurrentWidget(widget);
		}
		else
		{
			SideBar* b = getTabBar(pos,true);
			if (b)
				b->addTab(widget,text,icon);	
		}
	}

	void MainWindow::removeTab(QWidget* ti,TabPosition pos)
	{
		if (pos == CENTER)
		{
			int idx = tabs->indexOf(ti);
			if (idx == -1)
				return;
				
			tabs->removeTab(idx);
		}
		else
		{
			SideBar* b = getTabBar(pos,false);
			if (b)
				b->removeTab(ti);	
		}
	}
	
	void MainWindow::changeTabIcon(QWidget* ti,const QString & icon)
	{
		int idx = tabs->indexOf(ti);
		if (idx == -1)
			return;

		tabs->setTabIcon(idx,KIcon(icon));
	}
	
	void MainWindow::changeTabText(QWidget* ti,const QString & text)
	{
		int idx = tabs->indexOf(ti);
		if (idx == -1)
			return;

		tabs->setTabText(idx,text);
	}
	
	void MainWindow::loadSplitterState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("Splitters");
		if (vsplit)
		{
			QByteArray data;
			data = QByteArray::fromBase64(g.readEntry("vsplit",data));
			vsplit->restoreState(data);
		}

		if (hsplit)
		{
			QByteArray data;
			data = QByteArray::fromBase64(g.readEntry("hsplit",data));
			hsplit->restoreState(data);
		}
	}
	
	void MainWindow::saveSplitterState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("Splitters");
		if (vsplit)
		{
			QByteArray data = vsplit->saveState();
			g.writeEntry("vsplit",data.toBase64());
		}

		if (hsplit)
		{
			QByteArray data = hsplit->saveState();
			g.writeEntry("hsplit",data.toBase64());
		}
	}


	bool MainWindow::queryExit()
	{
		saveState(KGlobal::config());
		return true;
	}


	KPushButton* MainWindow::leftCornerButton()
	{
		if (!left_corner)
		{
			left_corner = new KPushButton(this);
			tabs->setCornerWidget(left_corner,Qt::TopLeftCorner);
		}
		return left_corner;
	}	

	KPushButton* MainWindow::rightCornerButton()
	{
		if (!right_corner)
		{
			right_corner = new KPushButton(this);
			tabs->setCornerWidget(right_corner,Qt::TopRightCorner);
		}
		return right_corner;
	}
	
	QWidget* MainWindow::currentTabPage()
	{
		return tabs->currentWidget();
	}
	
	void MainWindow::onCurrentTabChanged(int idx)
	{
		QWidget* page = tabs->widget(idx);
		currentTabPageChanged(page);
	}
}

#include "mainwindow.moc"
