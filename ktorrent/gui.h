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
#include <QStackedWidget>
#include <KXmlGuiWindow>
#include <util/constants.h>
#include <interfaces/guiinterface.h>

class KUrl;
class KAction;
class KToggleAction;
class KUrl;

namespace kt
{
	class Core;
	class PrefDialog;
	class StatusBar;
	class TrayIcon;
	class DBus;
	class TorrentActivity;
	class CentralWidget;
	

	class GUI : public KXmlGuiWindow,public GUIInterface
	{
		Q_OBJECT
	public:
		GUI();
		virtual ~GUI();

		DBus* getDBusInterface() {return dbus_iface;}
		
		// Stuff implemented from GUIInterface
		virtual KMainWindow* getMainWindow() {return this;}
		virtual void addPrefPage(PrefPageInterface* page);
		virtual void removePrefPage(PrefPageInterface* page);
		virtual void mergePluginGui(Plugin* p);
		virtual void removePluginGui(Plugin* p);
		virtual void dataScanStarted(ScanListener* listener);
		virtual void dataScanClosed(ScanListener* listener);
		virtual bool selectFiles(bt::TorrentInterface* tc,bool* start_torrent,const QString & group_hint,bool* skip_check);
		virtual void errorMsg(const QString & err);
		virtual void errorMsg(KIO::Job* j);
		virtual void infoMsg(const QString & info);
		virtual StatusBarInterface* getStatusBar();
		virtual void addActivity(Activity* act);
		virtual void removeActivity(Activity* act);
		virtual void setCurrentActivity(Activity* act);
		virtual TorrentActivityInterface* getTorrentActivity(); 
		
		/**
		* Create a XML GUI container (menu or toolbar)
		* @param name The name of the item
		* @return The widget
		*/
		QWidget* container(const QString & name);

		/// load a torrent
		void load(const KUrl & url);
		
		/// load a torrent silently
		void loadSilently(const KUrl & url);
		
	public slots:
		/**
		 * The paused state has changed
		 * @param paused 
		 */
		void onPausedStateChanged(bool paused);
		
		/// Update all actions
		void updateActions();
		
		/**
		 * Enable or disable the paste action
		 * @param on Set on
		 */
		void setPasteDisabled(bool on);

	private slots:
		void createTorrent();
		void openTorrent();
		void openTorrentSilently();
		void pauseQueue(bool pause);
		void startAllTorrents();
		void stopAllTorrents();
		void pasteURL();
		void paste();
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
		QTimer timer;
		kt::StatusBar* status_bar;
		TrayIcon* tray_icon;
		DBus* dbus_iface;
		TorrentActivity* torrent_activity;
		CentralWidget* central;
		PrefDialog* pref_dlg;
		
		KToggleAction* show_status_bar_action;
		KToggleAction* show_menu_bar_action;
		KAction* open_silently_action;
		KAction* start_all_action;
		KAction* stop_all_action;
		KAction* paste_url_action;
		KToggleAction* queue_pause_action;
		KAction* ipfilter_action;
		KAction* import_action;
		KAction* import_kde3_torrents_action;
		KAction* show_kt_action;
		KAction* paste_action;
		KAction* show_group_view_action;
	};
}

#endif
