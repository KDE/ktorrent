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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef INFOWIDGET_H
#define INFOWIDGET_H


#include "infowidgetbase.h"
#include <util/constants.h>

class KPopupMenu; 
class QString;
class QWidget;
class QHBoxLayout;


namespace kt
{
	class TorrentInterface;
	class TorrentFileInterface;
	class PeerView;
	class ChunkDownloadView;
	class TrackerView;
	class KTorrentMonitor;
	class IWFileTreeDirItem;

	using bt::Priority;

	class InfoWidget : public InfoWidgetBase
	{
		Q_OBJECT
	
	public:
		InfoWidget(bool seed = false, QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
		virtual ~InfoWidget();
		
		///Show PeerView in main window
		void showPeerView(bool show);
		///Show ChunkDownloadView in main window
		void showChunkView(bool show);
		///Show TrackerView in main window
		void showTrackerView(bool show);
		//change the priority of all children of a directory
	        void changePriority(QListViewItem* item, Priority newpriority);

	public slots:
		void changeTC(kt::TorrentInterface* tc);
		void update();
		void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
		void refreshFileTree(kt::TorrentInterface* tc);
	
		///preview slot
		void contextItem(int id);
    	virtual void maxRatio_returnPressed();
    	virtual void useLimit_toggled(bool);
	
	private:
		void fillFileTree();
		void readyPreview();
		void readyPercentage();
		void maxRatioUpdate();
		
	private:
		KTorrentMonitor* monitor;
		kt::TorrentInterface* curr_tc;
		IWFileTreeDirItem* multi_root;
		KPopupMenu* context_menu;
		QString preview_path;
		int preview_id;
		int first_id;
		int normal_id;
		int last_id;
		int dnd_id;
		QWidget* peer_page;
		PeerView* peer_view;
		ChunkDownloadView* cd_view;
		TrackerView* tracker_view;
		bool m_seed;
	};
}

#endif
