/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_GUI_HH
#define KT_GUI_HH

#include <KParts/MainWindow>
#include <KSharedConfig>
#include <QTimer>

#include <interfaces/guiinterface.h>
#include <util/constants.h>

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
    ~GUI() override;

    DBus *getDBusInterface()
    {
        return dbus_iface;
    }

    // Stuff implemented from GUIInterface
    KMainWindow *getMainWindow() override
    {
        return this;
    }
    void addPrefPage(PrefPageInterface *page) override;
    void removePrefPage(PrefPageInterface *page) override;
    void mergePluginGui(Plugin *p) override;
    void removePluginGui(Plugin *p) override;
    void errorMsg(const QString &err) override;
    void errorMsg(KIO::Job *j) override;
    void infoMsg(const QString &info) override;
    StatusBarInterface *getStatusBar() override;
    void addActivity(Activity *act) override;
    void removeActivity(Activity *act) override;
    TorrentActivityInterface *getTorrentActivity() override;
    QSize sizeHint() const override;

    bool event(QEvent *e) override;

    /**
     * Create a XML GUI container (menu or toolbar)
     * @param name The name of the item
     * @return The widget
     */
    QWidget *container(const QString &name);

    /// load a torrent
    void load(const QUrl &url);

    /// load a torrent silently
    void loadSilently(const QUrl &url);

public Q_SLOTS:
    /// Update all actions
    void updateActions();

    /**
     * Enable or disable the paste action
     * @param on Set on
     */
    void setPasteDisabled(bool on);

    /// Set the current activity
    void setCurrentActivity(Activity *act) override;

private Q_SLOTS:
    void createTorrent();
    void openTorrent(bool silently = false);
    void openTorrentSilently()
    {
        openTorrent(true);
    }
    void pasteURL();
    void paste();
    void showPrefDialog();
    void showIPFilter();
    void configureKeys();
    void configureToolbars() override;
    void newToolBarConfig();
    void import();
    void update();
    /// apply gui specific settings
    void applySettings();
    void showOrHide();
    void configureNotifications();
    void activePartChanged(KParts::Part *p);
    void quit();

private:
    void setupActions();

    void loadState(KSharedConfigPtr cfg);
    void saveState(KSharedConfigPtr cfg);
    bool queryClose() override;

private:
    Core *core;
    QTimer timer;
    kt::StatusBar *status_bar;
    TrayIcon *tray_icon;
    DBus *dbus_iface;
    TorrentActivity *torrent_activity;
    CentralWidget *central;
    PrefDialog *pref_dlg;
    KParts::PartManager *part_manager;

    KToggleAction *show_status_bar_action;
    KToggleAction *show_menu_bar_action;
    QAction *open_silently_action;

    QAction *paste_url_action;
    QAction *ipfilter_action;
    QAction *import_action;
    QAction *show_kt_action;
    QAction *paste_action;
};
}

#endif
