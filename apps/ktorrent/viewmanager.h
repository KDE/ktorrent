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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <interfaces/guiinterface.h>

class KConfig;
class KTabWidget;
class KTorrentView;
class KTorrent;

/**
	@author Joris Guisson <joris.guisson@gmail.com>
*/
class ViewManager : public QObject, public kt::CloseTabListener
{
	Q_OBJECT
public:
	ViewManager(QObject *parent = 0, const char *name = 0);
	virtual ~ViewManager();
	
	/// Create a new view
	KTorrentView* newView();
	
	/// Save all views
	void saveViewState(KConfig* cfg);
	
	/// Restore all views from configuration
	void restoreViewState(KConfig* cfg,KTorrent* ktor);
	
	/// Start all selected downloads in the current view
	void startDownloads();
	
	/// Stop all selected downloads in the current view
	void stopDownloads();
	
	/// Start all downloads in the current view
	void startAllDownloads();
	
	/// Stop all downloads in the current view
	void stopAllDownloads();
	
	/// Get the current torrent in the current view
	kt::TorrentInterface* getCurrentTC();
	
	/// Remove selected downloads in the current view
	void removeDownloads();
	
	/// Update the current view
	void update();
	
	/**
	 * Put the current selection of the current view in a list.
	 * @param sel The list to put it in
	 */
	void getSelection(QValueList<kt::TorrentInterface*> & sel);
	
	/// Get the current view
	KTorrentView* getCurrentView() {return current;}
	
	virtual bool closeAllowed(QWidget* tab);
	virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab);
	
	/// A group was renamed, modify all view which where showing this group
	void groupRenamed(kt::Group* g,KTabWidget* mtw);
	
	/// A group has been removed, close any tab showing it (in case it is the last tab, switch to All Torrents)
	void groupRemoved(kt::Group* g,KTabWidget* mtw,kt::GUIInterface* gui,kt::Group* all_group);
	
	/// Call updateActions on the current view
	void updateActions();
	
public slots:
	/// Add a torrent to all views
	void addTorrent(kt::TorrentInterface* tc);
	
	/// Remove a torernt from all views
	void removeTorrent(kt::TorrentInterface* tc);
	
	/// Enqueue/Dequeue selected torrents in current view
	void queueAction();
	
	/// Check data integrity of current torrent
	void checkDataIntegrity();
	
	/// The current tab has changed
	void onCurrentTabChanged(QWidget* w);

private:
	QValueList<KTorrentView*> views;
	KTorrentView* current;
};

#endif
