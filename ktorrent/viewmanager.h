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
#ifndef KT_VIEWMANAGER_HH
#define KT_VIEWMANAGER_HH

#include <QList>
#include <interfaces/guiinterface.h>
#include <ksharedconfig.h>


class QWidget;


namespace kt
{
	class View;
	class ViewModel;
	class Core;
	class GUI;
	class Group;
	enum ActionEnableFlags;

	class ViewManager : public QObject,public CloseTabListener
	{
		Q_OBJECT
	public:
		ViewManager(Group* all_group,GUI* gui,Core* core);
		virtual ~ViewManager();
		
		/// Create a new view
		View* newView(Core* core,QWidget* parent);
		
		/// Save all views
		void saveState(KSharedConfigPtr cfg);
		
		/// Restore all views from configuration
		void loadState(KSharedConfigPtr cfg);
		
		/// Start all selected downloads in the current view
		void startTorrents();
		
		/// Stop all selected downloads in the current view
		void stopTorrents();
		
		/// Start all downloads in the current view
		void startAllTorrents();
		
		/// Stop all downloads in the current view
		void stopAllTorrents();
		
		/// Enqueue or dequeue torrents
		void queueTorrents();

		/// Check the data of the selected torrent
		void checkData();

		/// Remove selected downloads in the current view
		void removeTorrents();
		
		/// Update the current view
		void update();
		
		/**
		 * Put the current selection of the current view in a list.
		 * @param sel The list to put it in
		 */
		void getSelection(QList<bt::TorrentInterface*> & sel);
		
		/// Get the current view
		View* getCurrentView() {return current;}

		/// Get the current torrent
		const bt::TorrentInterface* getCurrentTorrent() const;
		
		/// Get the current torrent (non const version)
		bt::TorrentInterface* getCurrentTorrent();

	public slots:
		void onCurrentTabChanged(QWidget* tab);
		void onCurrentGroupChanged(kt::Group* g);
		void onGroupRenamed(kt::Group* g);
		void onGroupRemoved(kt::Group* g);
		void onCurrentTorrentChanged(View* v,bt::TorrentInterface* tc);
		void onEnableActions(View* v,ActionEnableFlags flags);

	private:
		virtual bool closeAllowed(QWidget* w);
		virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab);
		
	signals:
		void enableActions(ActionEnableFlags flags);

	private:
		GUI* gui;
		View* current;
		QList<View*> views;
		Group* all_group; 
		ViewModel* model;
	};
}

#endif
