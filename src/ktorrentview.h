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


#ifndef _KTORRENTVIEW_H_
#define _KTORRENTVIEW_H_

#include <klistview.h>
#include <qtimer.h>

class KURL;
class KTorrentViewItem;
class KPopupMenu;

namespace bt
{
	class TorrentControl;
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

	/// Get the current TorrentControl object
	bt::TorrentControl* getCurrentTC();

	/// Enable or disable the debug view
	void setShowDebugView(bool yes);
public slots:
	void addTorrent(bt::TorrentControl* tc);
	void removeTorrent(bt::TorrentControl* tc);
	
private slots:
	void update();
	void onExecuted(QListViewItem* item);
	void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
	void startDownload();
	void stopDownload();
	void removeDownload();
	void onTrackerDown(bt::TorrentControl* tc);

signals:
	void torrentClicked(bt::TorrentControl* tc);
	void currentChanged(bt::TorrentControl* tc);
	void wantToRemove(bt::TorrentControl* tc);
		
private:
	QMap<bt::TorrentControl*,KTorrentViewItem*> items;
	QTimer timer;
	bool show_debug_view;
	KPopupMenu* menu;
	int stop_id,start_id,remove_id;
	KTorrentViewItem* curr;
};

#endif // _KTORRENTVIEW_H_
