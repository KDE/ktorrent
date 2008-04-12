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
#ifndef KT_GUI_HH
#define KT_GUI_HH

#include <QTimer>
#include <ideal/mainwindow.h>
#include <util/constants.h>
#include <interfaces/guiinterface.h>

class KUrl;
class KAction;
class KToggleAction;
class KUrl;

namespace kt
{
	class Core;
	class Group;
	class ViewManager;
	class PrefDialog;
	class StatusBar;
	class GroupView;
	class TrayIcon;
	class DBus;
	class View;
	class QueueManagerWidget;
	
	

	class GUI : public ideal::MainWindow,public GUIInterface
	{
		Q_OBJECT
	public:
		GUI();
		virtual ~GUI();

		// Stuff implemented from GUIInterface
		virtual KMainWindow* getMainWindow() {return this;}
		virtual void addTabPage(QWidget* page,const QString & icon,const QString & caption,CloseTabListener* ctl);
		virtual void removeTabPage(QWidget* page);
		virtual void addPrefPage(PrefPageInterface* page);
		virtual void removePrefPage(PrefPageInterface* page);
		virtual void mergePluginGui(Plugin* p);
		virtual void removePluginGui(Plugin* p);
		virtual void addToolWidget(QWidget* w,const QString & icon,const QString & caption,ToolDock dock);
		virtual void removeToolWidget(QWidget* w);
		virtual const bt::TorrentInterface* getCurrentTorrent() const;
		virtual void dataScan(bt::TorrentInterface* tc,bool auto_import,bool silently,const QString & dlg_caption);
		virtual bool selectFiles(bt::TorrentInterface* tc,bool* user,bool* start_torrent,const QString & group_hint);
		virtual void errorMsg(const QString & err);
		virtual void errorMsg(KIO::Job* j);
		virtual void infoMsg(const QString & info);
		virtual void currentTabPageChanged(QWidget* page);
		virtual StatusBarInterface* getStatusBar();

		/// load a torrent
		void load(const KUrl & url);
		
		/// load a torrent silently
		void loadSilently(const KUrl & url);

		/**
		 * Open a view
		 * @param group_name Name of group to show in view
		 */
		void openView(const QString & group_name);

		/**
		 * Called by the ViewManager when the current torrent has changed
		 * @param tc The torrent 
		 * */
		void currentTorrentChanged(bt::TorrentInterface* tc);
		
		/// Get the group view
		GroupView* getGroupView() {return group_view;}
	public slots:
		/**
		 * Open a view
		 * @param g The group to show in the view
		 * */
		void openView(kt::Group* g);
		
		/**
		 * Enable or disable some actions in the GUI
		 * @param flags Which actions to enable and disable
		 */
		void setActionsEnabled(ActionEnableFlags flags);
		
		/**
		 * The paused state has changed
		 * @param paused 
		 */
		void onPausedStateChanged(bool paused);

	private slots:
		void createTorrent();
		void openTorrent();
		void openTorrentSilently();
		void startTorrent();
		void stopTorrent();
		void removeTorrent();
		void queueTorrent();
		void pauseQueue(bool pause);
		void startAllTorrents();
		void startAllTorrentsCV();
		void stopAllTorrents();
		void stopAllTorrentsCV();
		void pasteURL();
		void paste();
		void checkData();
		void showPrefDialog();
		void showStatusBar();
		void showMenuBar();
		void showIPFilter();
		void configureKeys();
		void configureToolBars();
		void newToolBarConfig();
		void import();
		void update();
		/// apply gui specific settings
		void applySettings();
		void closeTab();
		void newView();
		void speedLimits();
		void showOrHide();
		void configureNotifications();		
		
		
	private:
		void setupActions();
		virtual void loadState(KSharedConfigPtr cfg);
		virtual void saveState(KSharedConfigPtr cfg);
		virtual bool queryExit();
		virtual bool queryClose();

	private:
		Core* core;
		ViewManager* view_man; 
		QTimer timer;
		kt::StatusBar* status_bar;
		GroupView* group_view;
		TrayIcon* tray_icon;
		DBus* dbus_iface;
		QueueManagerWidget* qm;

		KToggleAction* show_status_bar_action;
		KToggleAction* show_menu_bar_action;
		KAction* open_silently_action;
		KAction* start_action;
		KAction* stop_action;
		KAction* remove_action;
		KAction* start_all_action;
		KAction* start_all_cv_action;
		KAction* stop_all_action;
		KAction* stop_all_cv_action;
		KAction* paste_url_action;
		KAction* queue_action;
		KToggleAction* queue_pause_action;
		KAction* ipfilter_action;
		KAction* data_check_action;
		KAction* import_action;
		KAction* speed_limits_action;
		KAction* show_kt_action;

		PrefDialog* pref_dlg;
		QMap<QWidget*,CloseTabListener*> close_tab_map;
	};
}

#endif
