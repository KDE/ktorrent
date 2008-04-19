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


#ifndef _KTORRENT_H_
#define _KTORRENT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <newui/dmainwindow.h>
#include <qtimer.h>
#include <interfaces/guiinterface.h>

typedef QValueList<QCString> QCStringList;

class KAction;
class KToggleAction;
class KURL;
class KTorrentCore;
class KTorrentView;
class TrayIcon;
class SetMaxRate;
class KTorrentDCOP;
class QLabel;
class QListViewItem;
class KTorrentPreferences;
class ViewManager;


namespace kt
{
	class TorrentInterface;
	class Group;
	class GroupView;
}


/**
 * This class serves as the main window for KTorrent.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Joris Guisson <joris.guisson@gmail.com>
 * @version 0.1
 */
class KTorrent : public DMainWindow, public kt::GUIInterface
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 */
	KTorrent();

	/**
	 * Default Destructor
	 */
	virtual ~KTorrent();

	/// Open a view with the given group
	void openView(const QString & group_name);

	/// Get the core
	KTorrentCore & getCore() {return *m_core;}
	
	/**
	 * Apply the settings.
	 * @param change_port Wether or not to change the server port
	 */
	void applySettings(bool change_port = true);

	virtual void addTabPage(QWidget* page,const QIconSet & icon,
							const QString & caption,kt::CloseTabListener* ctl = 0);
	virtual void addTabPage(QWidget* page,const QPixmap & icon,
							const QString & caption,kt::CloseTabListener* ctl = 0);
	virtual void removeTabPage(QWidget* page);
	virtual void addPrefPage(kt::PrefPageInterface* page);
	virtual void removePrefPage(kt::PrefPageInterface* page);
	virtual void mergePluginGui(kt::Plugin* p);
	virtual void removePluginGui(kt::Plugin* p);
	virtual void addWidgetBelowView(QWidget* w,const QString & icon,const QString & caption);
	virtual void removeWidgetBelowView(QWidget* w);
	virtual void addToolWidget(QWidget* w,const QString & icon,const QString & caption,ToolDock dock);
	virtual void removeToolWidget(QWidget* w);
	virtual const kt::TorrentInterface* getCurrentTorrent() const;
	virtual KToolBar* addToolBar(const char* name);
	virtual void removeToolBar(KToolBar* tb);
	virtual KProgress* addProgressBarToStatusBar();
	virtual void removeProgressBarFromStatusBar(KProgress* p);
	
	QString	getStatusInfo();
	QString	getStatusTransfer();
	QString	getStatusSpeed();
	QString	getStatusDHT();
	QString getStatusFirewall();
	QCStringList getTorrentInfo(kt::TorrentInterface* tc);

public slots:
	/**
	 * Use this method to load whatever file/URL you have
	 */
	void load(const KURL& url);
	
	/**
	 * Does the same as load, but doesn't ask any questions
	*/
	void loadSilently(const KURL& url);
	
	/**
	 * Does the same as loadSilently, except accepts a directory to
	 * save to
	 */
	void loadSilentlyDir(const KURL& url, const KURL& savedir);
	
	/// Open a view with the given group
	void openView(kt::Group* g);

protected:
	/**
	 * This function is called when it is time for the app to save its
	 * properties for session management purposes.
	 */
	void saveProperties(KConfig *);

	/**
	 * This function is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 */
	void readProperties(KConfig *);


private slots:
	void fileOpen();
	void fileNew();
	void torrentPaste();
	void startDownload();
	void startAllDownloadsCurrentView();
	void startAllDownloads();
	void stopDownload();
	void stopAllDownloadsCurrentView();
	void stopAllDownloads();
	void showIPFilter();
	void removeDownload();
	void queueManagerShow();
	void optionsShowStatusbar();
	void optionsShowMenubar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void optionsPreferences();
	void newToolbarConfig();
	void changeStatusbar(const QString& text);
	void changeCaption(const QString& text);
	void currentTorrentChanged(kt::TorrentInterface* tc);
	void updatedStats();
	void urlDropped(QDropEvent*,QListViewItem*);
	void onUpdateActions(int flags);
	void groupChanged(kt::Group* g);
	void groupRenamed(kt::Group* g);
	void groupRemoved(kt::Group* g);
	void currentTabChanged(QWidget* w);
	void openDefaultView();
	void statusBarMsgExpired();
	void find();
	
private:
	void setupAccel();
	void setupActions();
	bool queryClose();
	bool queryExit();
	
	
	virtual void addWidgetInView(QWidget* w,kt::Position pos);
	virtual void removeWidgetFromView(QWidget* w);	
	virtual void closeTab();
	
private:
	kt::GroupView* m_group_view;
	ViewManager* m_view_man;
	KToggleAction *m_statusbarAction;
	KToggleAction* m_menubarAction;
	
	KTorrentCore* m_core;
	TrayIcon* m_systray_icon;
	SetMaxRate* m_set_max_upload_rate;
	SetMaxRate* m_set_max_download_rate;
	
	KTorrentDCOP* m_dcop;
	QTimer m_gui_update_timer;
	QTimer m_status_msg_expire;
	KTorrentPreferences* m_pref;

	QMap<QWidget*,kt::CloseTabListener*> m_tab_map;

	QLabel* m_statusInfo;
	QLabel* m_statusTransfer;
	QLabel* m_statusSpeed;
	QLabel* m_statusDHT;
	QLabel* m_statusFirewall;
	
	KAction* m_start;
	KAction* m_stop;
	KAction* m_remove;
	KAction* m_startall;
	KAction* m_startall_systray;
	KAction* m_stopall;
	KAction* m_stopall_systray;
	KAction* m_pasteurl;
	KAction* m_queuemgr; 
	KAction* m_queueaction;
	KAction* m_datacheck;
	KAction* m_ipfilter;
	KAction* m_find;
	
	KProgress* m_status_prog;
};

#endif // _KTORRENT_H_
