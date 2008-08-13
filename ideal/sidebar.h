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
#ifndef IDEAL_SIDEBAR_HHH
#define IDEAL_SIDEBAR_HHH

#include <QList>
#include <QStackedWidget>
#include <kmultitabbar.h>
#include <ksharedconfig.h>


class QSplitter;

namespace ideal
{
	class Box;

	/**
	 * Side bar in the main view, containg a bunch of tabs
	 * */
	class SideBar : public KMultiTabBar
	{
		Q_OBJECT
	public:
		struct Tab
		{
			QWidget* widget;
			int id;
			QString text;
			QString icon;
		};

		typedef QList<Tab>::iterator TabItr;

		SideBar(Box* box,QSplitter* split,KMultiTabBarPosition pos);
		virtual ~SideBar();

		/// Add a tab to the SideBar
		void addTab(QWidget* w,const QString & text,const QString & icon);

		/// Remove a tab from the SideBar
		void removeTab(QWidget* w);

		/// Save current status of sidebar, called at exit
		void saveState(KSharedConfigPtr cfg);

		/// Restore status from config, called at startup
		void loadState(KSharedConfigPtr cfg);

		/// Change the text of a tab
		void changeTabText(QWidget* w,const QString & text);

		/// Change the icon of a tab
		void changeTabIcon(QWidget* w,const QString & icon);
		
	private slots:
		void onTabClicked(int id);

	private:
		void shrink();
		void unshrink();
		TabItr findByWidget(QWidget* w);
		TabItr findById(int id);
		TabItr findByText(const QString & text);
		
	private:
		QSplitter* split;
		QStackedWidget* widget_stack;
		QList<Tab> tabs;
		int next_id;
		bool shrunken;
	};
}

#endif

