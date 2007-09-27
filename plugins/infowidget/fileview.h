/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#include <QTreeWidget>
#include <util/constants.h>

class KMenu;
class KSharedConfig;
template<class T> class KSharedPtr;
typedef KSharedPtr<KSharedConfig> KSharedConfigPtr;

namespace kt
{
	class TorrentInterface;
	class IWFileTreeDirItem;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class FileView : public QTreeWidget
	{
		Q_OBJECT
	public:
		FileView(QWidget *parent);
		virtual ~FileView();

		void update();
		void changeTC(kt::TorrentInterface* tc);

		void saveState(KSharedConfigPtr cfg);
		void loadState(KSharedConfigPtr cfg);

	private slots:
		void showContextMenu(const QPoint & p);
		void refreshFileTree(kt::TorrentInterface* tc);
		void onDoubleClicked(QTreeWidgetItem* item,int column);
		
	private:
		void fillFileTree();
		void readyPreview();
		void readyPercentage();
		void changePriority(QTreeWidgetItem* item, bt::Priority newpriority);
		void changePriority(bt::Priority newpriority);

	private slots:
		void open();
		void downloadFirst();
		void downloadLast();
		void downloadNormal();
		void doNotDownload();
		void deleteFiles();

	private:
		kt::TorrentInterface* curr_tc;
		IWFileTreeDirItem* multi_root;

		KMenu* context_menu;
		QAction* open_action;
		QAction* download_first_action;
		QAction* download_normal_action;
		QAction* download_last_action;
		QAction* dnd_action;
		QAction* delete_action;

		QString preview_path;
	};

}

#endif
