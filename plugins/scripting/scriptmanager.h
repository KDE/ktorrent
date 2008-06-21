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
class KActionCollection;

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
		ScriptManager(KActionCollection* ac,QWidget* parent);
		virtual ~ScriptManager();
		
	private slots:
		void onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
		
	signals:
		/**
		 * Emitted when the remove script action should be enabled.
		 * @param on Wether or not to enable the action
		 */
		void enableRemoveScript(bool on);
		
	private:
		ScriptModel* model;
		QListView* view;
		KToolBar* toolbar;
	};

}

#endif
