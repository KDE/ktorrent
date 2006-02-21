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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/


#ifndef _KTORRENT_H_
#define _KTORRENT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>
#include <qtimer.h>
#include <interfaces/guiinterface.h>



class KPrinter;
class KAction;
class KToggleAction;
class KURL;
class KTorrentCore;
class KTorrentView;
class TrayIcon;
class KTabWidget;
class KTorrentDCOP;
class QLabel;
class QListViewItem;
class KTorrentPreferences;



namespace kt
{
	class TorrentInterface;
	class ExpandableWidget;
}

/**
 * This class serves as the main window for KTorrent.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Joris Guisson <joris.guisson@gmail.com>
 * @version 0.1
 */
class KTorrent : public KMainWindow, public kt::GUIInterface
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


	KTorrentCore & getCore() {return *m_core;}
	
	/**
	 * Apply the settings.
	 * @param change_port Wether or not to change the server port
	 */
	void applySettings(bool change_port = true);

	virtual void addTabPage(QWidget* page,const QIconSet & icon,const QString & caption);
	virtual void removeTabPage(QWidget* page);
	virtual void addPrefPage(kt::PrefPageInterface* page);
	virtual void removePrefPage(kt::PrefPageInterface* page);
	virtual void mergePluginGui(kt::Plugin* p);
	virtual void removePluginGui(kt::Plugin* p);
	virtual void addWidgetBelowView(QWidget* w);
	virtual void removeWidgetBelowView(QWidget* w);
	virtual const kt::TorrentInterface* getCurrentTorrent() const;
	
	KTorrentView* getCurrentView();
	kt::PanelView getCurrentPanel();

public slots:
	/**
	 * Use this method to load whatever file/URL you have
	 */
	void load(const KURL& url);

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
	void startAllDownloads();
	void stopDownload();
	void stopAllDownloads();
	void removeDownload();
	void queueManagerShow();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void optionsPreferences();
	void newToolbarConfig();
	void changeStatusbar(const QString& text);
	void changeCaption(const QString& text);
	void currentDownloadChanged(kt::TorrentInterface* tc);
	void currentSeedChanged(kt::TorrentInterface* tc);
	void updatedStats();
	void urlDropped(QDropEvent*,QListViewItem*);
	void currentTabChanged(QWidget* tab);

private:
	void setupAccel();
	void setupActions();
	bool queryClose();
	bool queryExit();
	virtual void addWidgetInView(QWidget* w,kt::Position pos);
	virtual void removeWidgetFromView(QWidget* w);
	virtual void addWidgetInSeedView(QWidget* w,kt::Position pos);
	virtual void removeWidgetFromSeedView(QWidget* w);
	
	
private:
	KTorrentView *m_view;
	KTorrentView *m_seedView;
	KToggleAction *m_statusbarAction;
	KAction *m_start,*m_stop,*m_remove,*m_startall, *m_stopall, *m_pasteurl, *m_queuemgr;
	KTorrentCore* m_core;
	TrayIcon* m_systray_icon;
	KTabWidget* m_tabs;
	KTorrentDCOP* m_dcop;
	QTimer m_gui_update_timer;
	KTorrentPreferences* m_pref;
	kt::ExpandableWidget* m_exp;
	kt::ExpandableWidget* m_view_exp;
	kt::ExpandableWidget* m_seedView_exp;

	QLabel* m_statusInfo;
	QLabel* m_statusTransfer;
	QLabel* m_statusSpeed;
};

#endif // _KTORRENT_H_
