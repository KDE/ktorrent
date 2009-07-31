/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#ifndef KTSEARCHPLUGIN_H
#define KTSEARCHPLUGIN_H

#include <QList>
#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>
#include "searchenginelist.h"

namespace kt
{
	class SearchPrefPage;
	class SearchToolBar;
	class SearchActivity;

	/**
	@author Joris Guisson
	*/
	class SearchPlugin : public Plugin
	{
		Q_OBJECT
	public:
		SearchPlugin(QObject* parent, const QStringList& args);
		virtual ~SearchPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
		
		SearchEngineList* getSearchEngineList() const {return engines;}
		KAction* getBackAction() {return back_action;}
		int currentSearchEngine() const;
	private slots:
		void search(const QString & text,int engine,bool external);
		void preferencesUpdated();

	private:
		void setupActions();
		
	private:
		SearchActivity* activity;
		SearchPrefPage* pref;
		SearchToolBar* toolbar;
		SearchEngineList* engines;
		
		KAction* find_action;
		KAction* back_action;
		KAction* reload_action;
		KAction* search_action;
		KAction* copy_action;
		KAction* home_action;
	};

}

#endif
