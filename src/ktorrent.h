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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef _KTORRENT_H_
#define _KTORRENT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>



class KPrinter;
class KAction;
class KToggleAction;
class KURL;
class KTorrentCore;
class KTorrentView;
class TrayIcon;
class KTabWidget;
class SearchWidget;
class KTorrentDCOP;


namespace bt
{
	class TorrentControl;
}

/**
 * This class serves as the main window for KTorrent.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Joris Guisson <joris.guisson@gmail.com>
 * @version 0.1
 */
class KTorrent : public KMainWindow
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
	 */
	void applySettings();

public slots:
	/**
	 * Use this method to load whatever file/URL you have
	 */
	void load(const KURL& url);

protected:
	/**
	 * Overridden virtuals for Qt drag 'n drop (XDND)
	 */
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

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
	void fileSave();
	void startDownload();
	void stopDownload();
	void removeDownload();
	void optionsShowToolbar();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void optionsPreferences();
	void newToolbarConfig();
	void changeStatusbar(const QString& text);
	void changeCaption(const QString& text);
	void currentChanged(bt::TorrentControl* tc);
	void askAndSave(bt::TorrentControl* tc);

private:
	void setupAccel();
	void setupActions();
	void save(bt::TorrentControl* tc);
	bool queryClose();
	bool queryExit();
	
	
private:
	KTorrentView *m_view;
	KToggleAction *m_toolbarAction;
	KToggleAction *m_statusbarAction;
	KAction *m_start,*m_stop,*m_remove,*m_save;
	KTorrentCore* m_core;
	TrayIcon* m_systray_icon;
	KTabWidget* m_tabs;
	SearchWidget* m_search;
	KTorrentDCOP* m_dcop;
};

#endif // _KTORRENT_H_
