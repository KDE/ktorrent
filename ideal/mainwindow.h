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
#ifndef IDEAL_MAINWINDOW_HH
#define IDEAL_MAINWINDOW_HH

#include <kxmlguiwindow.h>
#include <ksharedconfig.h>

class QSplitter;
class KTabWidget;
class QToolButton;

namespace ideal
{
	class SideBar;
	class Box;
	
		
	/** 
	 * Class which controls the main window of ktorrent.
	 */
	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
	public:
		MainWindow();
		virtual ~MainWindow();

		enum TabPosition
		{
			CENTER,
			LEFT,
			RIGHT,
			BOTTOM
		};
		
		/// Add a widget in a new tab
		void addTab(QWidget* widget,const QString & text,const QString & icon,TabPosition pos = CENTER);

		/// Remove a tab
		void removeTab(QWidget* w,TabPosition pos = CENTER);

		/// Load the state of all the windows 
		virtual void loadState(KSharedConfigPtr cfg);

		/// Save the window state
		virtual void saveState(KSharedConfigPtr cfg);

		/// Change the icon of a tab
		void changeTabIcon(QWidget* ti,const QString & icon);

		/// Change the text of tab
		void changeTabText(QWidget* ti,const QString & text);

		/// Get the left corner button of the tabwidget (if it doesn't exist it will be created)
		QToolButton* leftCornerButton();	

		/// Get the right corner button of the tabwidget (if it doesn't exist it will be created)
		QToolButton* rightCornerButton();

		/// Get the current tab page of the tab widget
		QWidget* currentTabPage();

		/// The current tab page has changed
		virtual void currentTabPageChanged(QWidget* ) {}
		
		/**
		 * Create a XML GUI container (menu or toolbar)
		 * @param name The name of the item
		 * @return The widget
		 */
		QWidget* container(const QString & name);
		
		
	protected:
		virtual bool queryExit();

	private:
		SideBar* getTabBar(TabPosition pos,bool create);
		void createDockArea(TabPosition pos);
		void setupSplitters();
		void saveSplitterState(KSharedConfigPtr cfg);
		void loadSplitterState(KSharedConfigPtr cfg);

	private slots:
		void onCurrentTabChanged(int idx);

	private:
		KTabWidget* tabs;
		SideBar* left;
		SideBar* right;
		SideBar* bottom;
		QSplitter* hsplit;
		Box* hbox;
		QSplitter* vsplit;
		Box* vbox;
		QToolButton* left_corner;
		QToolButton* right_corner;
	};
}


#endif
