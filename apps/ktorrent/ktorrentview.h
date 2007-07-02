/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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


#ifndef _KTORRENTVIEW_H_
#define _KTORRENTVIEW_H_

#include <klistview.h>

typedef QValueList<QCString> QCStringList;

class KURL;
class KTorrentViewItem;
class KPopupMenu;
class KTorrentCore;
class KTorrentViewMenu;
class ScanDialog;
class QString;

namespace kt
{
	class TorrentInterface;
	class Group;
}

using namespace bt;

/**
 * List view which shows information about torrents.
 */
class KTorrentView : public KListView
{
	Q_OBJECT
public:
	/**
	 * Default constructor
	 */
	KTorrentView(QWidget *parent);

	/**
	 * Destructor
	 */
	virtual ~KTorrentView();
	
	/// Update the caption, so the correct number of running torrents is shown in the tab
	void updateCaption();
	
	/// Get the current group
	const kt::Group* getCurrentGroup() const {return current_group;}

	/// Get the current TorrentInterface object
	kt::TorrentInterface* getCurrentTC();
	
	/// Save the views settings
	void saveSettings(KConfig* cfg,int idx);
	
	/// Load the views settings
	void loadSettings(KConfig* cfg,int idx);
	
	/**
	 * Put the current selection in a list.
	 * @param sel The list to put it in
	 */
	void getSelection(QValueList<kt::TorrentInterface*> & sel);
	
	/**
	 * Add the current selection to a group. 
	 * @param g The group
	 */
	void addSelectionToGroup(kt::Group* g);
	
	/**
	 * Reimplemented for header context menu
	 */
	bool eventFilter(QObject* watched, QEvent* e);
	
	/**
	 * Is column visible?
	 */
	bool columnVisible(int index);
	
	/**
	 * Setup the view columns, based upon the current group
	 * This will hide some columns for uploads only groups.
	 */
	void setupViewColumns();
	
public slots:
	void setCurrentGroup(kt::Group* group);
	void addTorrent(kt::TorrentInterface* tc);
	void removeTorrent(kt::TorrentInterface* tc);
	void update();
	void startDownloads();
	void stopDownloads();
	void startAllDownloads();
	void stopAllDownloads();
	void manualAnnounce();
	void previewFiles();
	void removeDownloads();
	void removeDownloadsAndData();
	void onSelectionChanged();
	void queueSlot();
	void checkDataIntegrity();
	void removeFromGroup();
	void showAddPeersWidget();
	void openOutputDirectory();
	void openTorXDirectory();
	void setDownloadLocationSlot();
	void dhtSlot();
	void utPexSlot();
	void speedLimits();

private slots:
	void onExecuted(QListViewItem* item);
	void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
	void onColumnVisibilityChange(int);
	void gsmItemActived(const QString & group);
	
	
signals:
	void torrentClicked(kt::TorrentInterface* tc);
	void currentChanged(kt::TorrentInterface* tc);
	void wantToRemove(kt::TorrentInterface* tc,bool data_to);
	void wantToStop(kt::TorrentInterface* tc,bool user);
	void wantToStart(kt::TorrentInterface* tc);
	void viewChange(kt::TorrentInterface* tc);
	void updateActions(bool can_start,bool can_stop,bool can_remove,bool can_scan);
	void queue(kt::TorrentInterface* tc);
	void needsDataCheck(kt::TorrentInterface* tc);
	void updateGroupsSubMenu(KPopupMenu* gsm);
	void groupsSubMenuItemActivated(KTorrentView* v,const QString & group);

private:
	bool acceptDrag(QDropEvent* event) const;
	int getNumRunning();
	bool startDownload(kt::TorrentInterface* tc);
	void stopDownload(kt::TorrentInterface* tc);
	void showStartError();
	virtual QDragObject* dragObject();
	void setupColumns();
	void insertColumn(QString label, Qt::AlignmentFlags);
	void columnHide(int index);
	void columnShow(int index);	
		
private:
	QMap<kt::TorrentInterface*,KTorrentViewItem*> items;
	KTorrentViewMenu* menu;
	KPopupMenu* m_headerMenu;
	kt::Group* current_group;
	Uint32 running;
	Uint32 total;
};

#endif // _KTORRENTVIEW_H_
