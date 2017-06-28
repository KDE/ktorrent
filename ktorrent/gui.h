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
#include <KParts/MainWindow>
#include <KSharedConfig>

#include <util/constants.h>
#include <interfaces/guiinterface.h>

class QAction;
class KToggleAction;

namespace kt
{
    class Core;
    class PrefDialog;
    class StatusBar;
    class TrayIcon;
    class DBus;
    class TorrentActivity;
    class CentralWidget;


    class GUI : public KParts::MainWindow, public GUIInterface
    {
        Q_OBJECT
    public:
        GUI();
        ~GUI();

        DBus* getDBusInterface() {return dbus_iface;}

        // Stuff implemented from GUIInterface
        KMainWindow* getMainWindow() {return this;}
        void addPrefPage(PrefPageInterface* page);
        void removePrefPage(PrefPageInterface* page);
        void mergePluginGui(Plugin* p);
        void removePluginGui(Plugin* p);
        void errorMsg(const QString& err);
        void errorMsg(KIO::Job* j);
        void infoMsg(const QString& info);
        StatusBarInterface* getStatusBar();
        void addActivity(Activity* act);
        void removeActivity(Activity* act);
        TorrentActivityInterface* getTorrentActivity();
        QSize sizeHint() const;

        bool event(QEvent *e);

        /**
        * Create a XML GUI container (menu or toolbar)
        * @param name The name of the item
        * @return The widget
        */
        QWidget* container(const QString& name);

        /// load a torrent
        void load(const QUrl& url);

        /// load a torrent silently
        void loadSilently(const QUrl& url);

    public slots:
        /// Update all actions
        void updateActions();

        /**
         * Enable or disable the paste action
         * @param on Set on
         */
        void setPasteDisabled(bool on);

        /// Set the current activity
        void setCurrentActivity(Activity* act);

    private slots:
        void createTorrent();
        void openTorrent(bool silently = false);
        void openTorrentSilently(){openTorrent(true);}
        void pasteURL();
        void paste();
        void showPrefDialog();
        void showIPFilter();
        void configureKeys();
        void configureToolbars();
        void newToolBarConfig();
        void import();
        void update();
        /// apply gui specific settings
        void applySettings();
        void showOrHide();
        void configureNotifications();
        void activePartChanged(KParts::Part* p);
        void quit();

    private:
        void setupActions();

        void loadState(KSharedConfigPtr cfg);
        void saveState(KSharedConfigPtr cfg);
        bool queryClose();

    private:
        Core* core;
        QTimer timer;
        kt::StatusBar* status_bar;
        TrayIcon* tray_icon;
        DBus* dbus_iface;
        TorrentActivity* torrent_activity;
        CentralWidget* central;
        PrefDialog* pref_dlg;
        KParts::PartManager* part_manager;

        KToggleAction* show_status_bar_action;
        KToggleAction* show_menu_bar_action;
        QAction * open_silently_action;

        QAction * paste_url_action;
        QAction * ipfilter_action;
        QAction * import_action;
        QAction * show_kt_action;
        QAction * paste_action;
    };
}

#endif
