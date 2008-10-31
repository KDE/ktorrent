/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KTSCRIPTMANAGER_H
#define KTSCRIPTMANAGER_H

#include <QListView>

class KToolBar;
class KMenu;
class KActionCollection;

namespace Kross
{
	class Action;
}

namespace kt
{
	class ScriptModel;

	/**
		Widget to display all scripts.
	*/
	class ScriptManager : public QWidget
	{
		Q_OBJECT
	public:
		ScriptManager(ScriptModel* model,KActionCollection* ac,QWidget* parent);
		virtual ~ScriptManager();
		
		/// Get all selected scripts
		QModelIndexList selectedScripts();
		
		/// Update all actions and make sure they are properly enabled or disabled
		void updateActions(const QModelIndexList & selected);
		
	private slots:
		void onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
		void showContextMenu(const QPoint& p);
		void dataChanged(const QModelIndex & f,const QModelIndex & to);
		
	signals:
		void enableRemoveScript(bool on);
		void enableRunScript(bool on);
		void enableStopScript(bool on);
		void enableProperties(bool on);
		void enableConfigure(bool on);
		
	private:
		ScriptModel* model;
		QListView* view;
		KToolBar* toolbar;
		KMenu* context_menu;
	};

}

#endif
