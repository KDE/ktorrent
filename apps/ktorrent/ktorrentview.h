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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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

namespace kt
{
	class TorrentInterface;
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
	KTorrentView(QWidget *parent, bool seed_view = false);

	/**
	 * Destructor
	 */
	virtual ~KTorrentView();

	/// Get the current TorrentInterface object
	kt::TorrentInterface* getCurrentTC();

	QCStringList getTorrentInfo(kt::TorrentInterface* tc);

	/// Enable or disable the debug view
	void setShowDebugView(bool yes);
	
	/// Save the views settings
	void saveSettings();
	
public slots:
	void addTorrent(kt::TorrentInterface* tc);
	void removeTorrent(kt::TorrentInterface* tc);
	void update();
	void torrentFinished(kt::TorrentInterface* tc);
	void startDownloads();
	void stopDownloads();
	void manualAnnounce();
	void previewFiles();
	void removeDownloads();
	void onSelectionChanged();
	void queueSlot();
	void checkDataIntegrity();

private slots:
	void onExecuted(QListViewItem* item);
	void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
	
	
signals:
	void torrentClicked(kt::TorrentInterface* tc);
	void currentChanged(kt::TorrentInterface* tc);
	void wantToRemove(kt::TorrentInterface* tc,bool data_to);
	void wantToStop(kt::TorrentInterface* tc,bool user);
	void wantToStart(kt::TorrentInterface* tc);
	void viewChange(kt::TorrentInterface* tc);
	void updateActions(bool can_start,bool can_stop,bool can_remove,bool can_scan);
	void queue(kt::TorrentInterface* tc);

private:
	bool acceptDrag(QDropEvent* event) const;
	int getNumRunning();
	void makeMenu();
		
private:
	bool m_seedView;
	QMap<kt::TorrentInterface*,KTorrentViewItem*> items;
	bool show_debug_view;
	KPopupMenu* menu;
	int stop_id, start_id, remove_id, preview_id, announce_id, queue_id,scan_id;
};

#endif // _KTORRENTVIEW_H_
