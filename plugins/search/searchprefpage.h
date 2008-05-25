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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTSEARCHPREFPAGE_H
#define KTSEARCHPREFPAGE_H


#include <interfaces/prefpageinterface.h>
#include "searchpref.h"

#include <qstring.h>

namespace kt
{	
	class SearchPlugin;
	class SearchEngineList;
	
	class SearchPrefPageWidget : public SEPreferences
	{
		Q_OBJECT
	public:
		SearchPrefPageWidget(QWidget *parent = 0);
	
		bool apply();
		void saveSearchEngines();
		void updateList(QString& source);
		
		void updateSearchEngines(const SearchEngineList & se);
		
	public slots:
		virtual void btnUpdate_clicked();
		void customToggled(bool toggled);
	
	private slots:
		void addClicked();
		void removeClicked();
		void addDefaultClicked();
		void removeAllClicked();
	}; 

	/**
	@author Joris Guisson
	*/
	class SearchPrefPage : public PrefPageInterface
	{
	public:
		SearchPrefPage(SearchPlugin* plugin);
		virtual ~SearchPrefPage();

		virtual bool apply();
		virtual void createWidget(QWidget* parent);
		virtual void updateData();
		virtual void deleteWidget();

	private:
		SearchPrefPageWidget* widget;
		SearchPlugin* m_plugin;
	};

}

#endif
