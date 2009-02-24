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

#ifndef SEARCHACTIVITY_H
#define SEARCHACTIVITY_H

#include <QList>
#include <kurl.h>
#include <ktabwidget.h>
#include <kaction.h>
#include <interfaces/activity.h>

namespace kt
{
	class SearchWidget;
	class SearchEngineList;
	class SearchPlugin;

	class SearchActivity : public kt::Activity
	{
		Q_OBJECT
	public:
		SearchActivity(SearchPlugin* sp,QWidget* parent);
		virtual ~SearchActivity();
		
		/// Add a SearchWidget
		void search(const QString & text,int engine);
		
		/// Save all current searches
		void saveCurrentSearches();
		
		/// Load current searches
		void loadCurrentSearches();
		
	public slots:
		void find();
		void back();
		void reload();
		void search();
		void copy();
		void home();
		void openNewTab(const KUrl & url);
		void currentTabChanged(int idx);
		void closeTab();
		void openTab();
		void setTabTitle(SearchWidget* sw,const QString & title);
		
	private:
		SearchWidget* newSearchWidget(const QString & text);
		
	private:
		KTabWidget* tabs;
		QList<SearchWidget*> searches;
		SearchPlugin* sp;
	};
}

#endif // SEARCHACTIVITY_H
