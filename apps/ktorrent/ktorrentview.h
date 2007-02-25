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
class ScanDialog;
class QString;

namespace kt
{
	class TorrentInterface;
	class Group;
}


using namespace bt;

/**
 * This is the main view class for KTorrent.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * This ktorrent uses an HTML component as an example.
 *
 * @short Main view
 * @author Joris Guisson <joris.guisson@gmail.com>
 * @version 0.1
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

	/// Get the current TorrentInterface object
	kt::TorrentInterface* getCurrentTC();

	static QCStringList getTorrentInfo(kt::TorrentInterface* tc);
	
	/// Save the views settings
	void saveSettings();
	
	/// Load the views settings
	void loadSettings();
	
	/**
	 * Put the current selection in a list.
	 * @param sel The list to put it in
	 */
	void getSelection(QPtrList<kt::TorrentInterface> & sel);
	
	/// Get the groups sub menu
	KPopupMenu* getGroupsSubMenu() {return groups_sub_menu;}
	
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
	
public slots:
	void setCurrentGroup(kt::Group* group);
	void addTorrent(kt::TorrentInterface* tc);
	void removeTorrent(kt::TorrentInterface* tc);
	void update();
	void torrentFinished(kt::TorrentInterface* tc);
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
	void dhtSlot();
	void utPexSlot();

private slots:
	void onExecuted(QListViewItem* item);
	void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
	void onColumnVisibilityChange(int);
	
	
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

private:
	bool acceptDrag(QDropEvent* event) const;
	int getNumRunning();
	void makeMenu();
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
	KPopupMenu* menu;
	KPopupMenu* groups_sub_menu;
	KPopupMenu* dirs_sub_menu;
	KPopupMenu* peer_sources_menu;
	KPopupMenu* m_headerMenu;
	int stop_id;
	int start_id;
	int remove_id;
	int remove_all_id;
	int preview_id;
	int announce_id;
	int queue_id;
	int scan_id;
	int remove_from_group_id;
	int add_to_group_id;
	int add_peer_id;
	int dirs_id;
	int outputdir_id;
	int torxdir_id;
	int peer_sources_id;
	int dht_id;
	int ut_pex_id;
	kt::Group* current_group;
};

#endif // _KTORRENTVIEW_H_
