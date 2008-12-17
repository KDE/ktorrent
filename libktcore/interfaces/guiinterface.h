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
#ifndef KTGUIINTERFACE_H
#define KTGUIINTERFACE_H

#include <qlist.h>
#include <ktcore_export.h>

class QWidget;
class QString;
class QProgressBar;
class KMainWindow;

namespace KIO
{
	class Job;
}

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class PrefPageInterface;
	class Plugin;
	class GUIInterface;

	enum Position
	{
		LEFT, ///< New widgets will be added to the left of the old
		RIGHT, ///< New widgets will be added to the right of the old
		ABOVE, ///< New widgets will be added above the old
		BELOW  ///< New widgets will be added below the old
	};
	
	/**
	 * Small interface for classes who want to know when
	 * current torrent in the gui changes.
	 */
	class KTCORE_EXPORT ViewListener
	{
	public:
		ViewListener() {}
		virtual ~ViewListener() {}
		
		virtual void currentTorrentChanged(bt::TorrentInterface* tc) = 0;
	};
	
	/**
	 * Plugins wanting to add closeable tabs, should implement this interface.
	 * That way they can be notified of close requests.
	 * Not providing this interface in addTabPage means the tab cannot be closed.
	*/
	class KTCORE_EXPORT CloseTabListener
	{
	public:
		CloseTabListener() {}
		virtual ~CloseTabListener() {}

		/// By default all tabs can be closed, but this can be overridden
		virtual bool closeAllowed(QWidget* ) {return true;}
		
		/// THe close button was pressed for this tab, please remove it from the GUI
		virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab) = 0;
	};
	
	/**
	 * Plugins wanting to know when the current tab changes, should implement this interface.
	 */
	class KTCORE_EXPORT CurrentTabPageListener
	{
	public:
		virtual ~CurrentTabPageListener() {}
		
		/// The current tab page has changed
		virtual void currentTabPageChanged(QWidget* page) = 0;
	};

	/**
	 * Base class for the status bar
	 * */
	class KTCORE_EXPORT StatusBarInterface
	{
	public:
		virtual ~StatusBarInterface() {}

		/// Show a message on the statusbar for some period of time
		virtual void message(const QString & msg) = 0;


		/// Create a progress bar and put it on the right side of the statusbar
		virtual QProgressBar* createProgressBar() = 0;

		/// Remove a progress bar created with createProgressBar (pb will be deleteLater'ed)
		virtual void removeProgressBar(QProgressBar* pb) = 0;
	};
	
	/**
	 * @author Joris Guisson
	 * @brief Interface to modify the GUI
	 *
	 * This interface allows plugins and others to modify the GUI.
	*/
	class KTCORE_EXPORT GUIInterface
	{
		QList<ViewListener*> listeners;
		QList<CurrentTabPageListener*> ctp_listeners;
	public:
		GUIInterface();
		virtual ~GUIInterface();

		/// Get a pointer to the main window
		virtual KMainWindow* getMainWindow() = 0;

		/// Add a view listener.
		void addViewListener(ViewListener* vl);

		/// Remove a view listener
		void removeViewListener(ViewListener* vl);
		
		/// Add a current tab page listener
		void addCurrentTabPageListener(CurrentTabPageListener* ctpl);
		
		/// Remove a current tab page listener
		void removeCurrentTabPageListener(CurrentTabPageListener* ctpl);
		
		/**
		 * Add a new tab page to the GUI
		 * @param page The widget
		 * @param icon Icon for the tab
		 * @param caption Text on the tab
		 * @param tooltip Tooltip for the tab
		 * @param ctl For closeable tabs this pointer should be set
		 */
		virtual void addTabPage(QWidget* page,const QString & icon,const QString & caption,const QString & tooltip,CloseTabListener* ctl = 0) = 0;

		/**
		 * Remove a tab page, does nothing if the page
		 * isn't added. Does not delete the widget.
		 * @param page The page
		 */
		virtual void removeTabPage(QWidget* page) = 0;

		/**
		 * Add a page to the preference dialog.
		 * @param page The page
		 */
		virtual void addPrefPage(PrefPageInterface* page) = 0;

		
		/**
		 * Remove a page from the preference dialog.
		 * @param page The page
		 */
		virtual void removePrefPage(PrefPageInterface* page) = 0;

		/**
		 * Merge the GUI of a plugin.
		 * @param p The Plugin
		 */
		virtual void mergePluginGui(Plugin* p) = 0;

		/**
		 * Remove the GUI of a plugin.
		 * @param p The Plugin
		 */
		virtual void removePluginGui(Plugin* p) = 0;
		
		enum ToolDock
		{
			DOCK_LEFT,
			DOCK_RIGHT,
			DOCK_BOTTOM
		};
		
		/**
		 * Add a tool widget.
		 * @param w The widget
		 * @param icon Name of icon to use
		 * @param caption The caption to use
		 * @param tooltip Tooltip for the tool widget
		 * @param dock Where to dock the widget
		 */
		virtual void addToolWidget(QWidget* w,const QString & icon,const QString & caption,const QString & tooltip,ToolDock dock) = 0;
		
		/**
		 * Remove a tool widget.
		 * @param w The widget
		 */
		virtual void removeToolWidget(QWidget* w) = 0;

		/// Get the current torrent.
		virtual const bt::TorrentInterface* getCurrentTorrent() const = 0;
		
		/// Get the current torrent 
		virtual bt::TorrentInterface* getCurrentTorrent() = 0;
	
		/// Show a scan dialog, and start the data scan
		virtual void dataScan(bt::TorrentInterface* tc,bool auto_import,bool silently,const QString & dlg_caption) = 0;

		/// Select the files to download, return false if users cancels
		virtual bool selectFiles(bt::TorrentInterface* tc,bool* user,bool* start_torrent,const QString & group_hint,bool* skip_check) = 0;

		/// Show an error message box
		virtual void errorMsg(const QString & err) = 0;

		/// Show an error message for a KIO job which went wrong
		virtual void errorMsg(KIO::Job* j) = 0;

		/// Show an information dialog
		virtual void infoMsg(const QString & info) = 0;

		/// Get the status bar
		virtual StatusBarInterface* getStatusBar() = 0;
		
		/**
		 * Set the icon of a tab
		 * @param tab 
		 * @param icon 
		 */
		virtual void setTabIcon(QWidget* tab,const QString & icon) = 0;
		
		/**
		 * Set the text of a tab
		 * @param tab 
		 * @param icon 
		 */
		virtual void setTabText(QWidget* tab,const QString & text) = 0;
		
		/**
		 * Set the current tab page.
		 * @param tab 
		 */
		virtual void setCurrentTab(QWidget* tab) = 0;
		
		/**
		 * Get the current tab page
		 * @return The current tab page
		 */
		virtual QWidget* getCurrentTab() = 0;

	protected:
		/**
		 * Notifies all view listeners of the change in the current downloading TorrentInterface
		 * @param tc Pointer to current TorrentInterface
		 */
		void notifyViewListeners(bt::TorrentInterface* tc);
		
		/**
		 * Notify current tab page listeners that the current tab page has changed
		 * @param page The page
		 */
		void notifyCurrentTabPageListeners(QWidget* page);
	};

}

#endif
