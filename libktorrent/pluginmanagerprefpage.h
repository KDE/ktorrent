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
#ifndef KTPLUGINMANAGERPREFPAGE_H
#define KTPLUGINMANAGERPREFPAGE_H

#include <qobject.h>
#include <interfaces/prefpageinterface.h>

class QListViewItem;
class PluginManagerWidget;

namespace kt
{
	class PluginManager;
	class LabelViewItem;

	/**
	 * @author Joris Guisson
	 *
	 * Pref page which allows to load and unload plugins.
	*/
	class PluginManagerPrefPage : public QObject,public PrefPageInterface
	{
		Q_OBJECT
	public:
		PluginManagerPrefPage(PluginManager* pman);
		virtual ~PluginManagerPrefPage();

		virtual bool apply();
		virtual void createWidget(QWidget* parent);
		virtual void updateData();
		virtual void deleteWidget();
		
		void updatePluginList();
		
	private slots:
		void onLoad();
		void onUnload();
		void onLoadAll();
		void onUnloadAll();
		void onCurrentChanged(LabelViewItem* item);
		
	private:
		void updateAllButtons();
		
	private:
		PluginManager* pman;
		PluginManagerWidget* pmw;
	};

}

#endif
