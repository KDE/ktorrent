/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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

#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <ktoolbar.h>
		
class KComboBox;
class KPushButton;

namespace kt
{
	class SearchEngineList;

	/**
		Holds all widgets of the toolbar of the search plugin.
	*/
	class SearchTab : public QObject
	{
		Q_OBJECT
	
	public:
		SearchTab(KToolBar* toolbar);
		virtual ~SearchTab();
		
		/// Get the tool bar
		KToolBar* getToolBar() {return m_tool_bar;}
	
		/// Update the search engine list
		void updateSearchEngines(const SearchEngineList & sl);
		
		/// Save settings like current search engine
		void saveSettings();
			
	protected slots:
		void clearButtonPressed();
		void searchNewTabPressed();
		void searchBoxReturn(const QString & str);
		void textChanged(const QString & str);
		
	signals:
		/// Emitted when the user presses enter or clicks search
		void search(const QString & text,int engine,bool external);
		
	private:
		void loadSearchHistory();
		void saveSearchHistory();
	
	private:
		KToolBar* m_tool_bar;
		KComboBox* m_search_text;
		KComboBox* m_search_engine;
		KPushButton* m_clear_button;
		KPushButton* m_search_new_tab;
	};
}

#endif

