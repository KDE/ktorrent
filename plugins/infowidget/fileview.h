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
#ifndef KTFILEVIEW_H
#define KTFILEVIEW_H

#include <klistview.h>
#include <util/constants.h>
#include <qtimer.h>

namespace kt
{
	class TorrentInterface;
	class IWFileTreeDirItem;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class FileView : public KListView
	{
		Q_OBJECT
	public:
		FileView(QWidget *parent = 0, const char *name = 0);
		virtual ~FileView();

		void update();
		void changeTC(kt::TorrentInterface* tc);
	private slots:
		void contextItem(int id);
		void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
		void refreshFileTree(kt::TorrentInterface* tc);
		void onDoubleClicked(QListViewItem* item,const QPoint & ,int );
		void fillTreePartial();
		
	private:
		void fillFileTree();
		void readyPreview();
		void readyPercentage();
		void changePriority(QListViewItem* item, bt::Priority newpriority);
		
	private:
		kt::TorrentInterface* curr_tc;
		IWFileTreeDirItem* multi_root;
		bool pending_fill;
		KPopupMenu* context_menu;
		QString preview_path;
		QTimer fill_timer;
		int preview_id;
		int first_id;
		int normal_id;
		int last_id;
		int dnd_keep_id;
		int dnd_throw_away_id;

		int next_fill_item;
	};

}

#endif
