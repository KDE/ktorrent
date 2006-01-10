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
#ifndef KTGUIINTERFACE_H
#define KTGUIINTERFACE_H

#include <qptrlist.h>

class QWidget;
class QIconSet;
class QString;

namespace kt
{
	class PrefPageInterface;
	class Plugin;
	class TorrentInterface;

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
	class ViewListener
	{
	public:
		virtual void currentChanged(TorrentInterface* tc) = 0;
	};
	
	/**
	 * @author Joris Guisson
	 * @brief Interface to modify the GUI
	 *
	 * This interface allows plugins and others to modify the GUI.
	*/
	class GUIInterface
	{
		QPtrList<ViewListener> listeners;
	public:
		GUIInterface();
		virtual ~GUIInterface();


		/// Add a view listener.
		void addViewListener(ViewListener* vl);

		/// Remove a view listener
		void removeViewListener(ViewListener* vl);
		
		/**
		 * Add a new tab page to the GUI
		 * @param page The widget
		 * @param icon Icon for the tab
		 * @param caption Text on the tab
		 */
		virtual void addTabPage(QWidget* page,const QIconSet & icon,const QString & caption) = 0;

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
		 * Change the statusbar message.
		 * @param msg The new message
		 */
		virtual void changeStatusbar(const QString& msg) = 0;

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
		
		/**
		 * Embed a widget in the view in the mainview.
		 * The view and the new widget will be separated by a separator.
		 * @param w The widget
		 * @param pos How the widget will be positioned against the already present widgets
		 */
		virtual void addWidgetInView(QWidget* w,Position pos) = 0;

		/**
		 * Remove a widget added with addWidgetInView.
		 * The widget will be reparented to 0.
		 * @param w The widget 
		 */
		virtual void removeWidgetFromView(QWidget* w) = 0;
		
		/**
		 * Add a widget below the view.
		 * @param w The widget
		 */
		virtual void addWidgetBelowView(QWidget* w) = 0;
		
		/**
		 * Remove a widget, which was added below the view.
		 * @param w The widget
		 */
		virtual void removeWidgetBelowView(QWidget* w) = 0;

		/// Get the current torrent.
		virtual const TorrentInterface* getCurrentTorrent() const = 0;
	protected:
		/**
		 * Notifies all view listeners of the change in the current TorrentInterface
		 * @param tc Pointer to current TorrentInterface
		 */
		void notifyViewListeners(TorrentInterface* tc);
	};

}

#endif
