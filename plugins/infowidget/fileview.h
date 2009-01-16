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

#include <QTreeView>
#include <util/constants.h>
#include <ksharedconfig.h>

class KMenu;
class QSortFilterProxyModel;

namespace bt
{
	class TorrentInterface;
	class TorrentFileInterface;
}

namespace kt
{
	class TorrentFileModel;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class FileView : public QTreeView
	{
		Q_OBJECT
	public:
		FileView(QWidget *parent);
		virtual ~FileView();

		void changeTC(bt::TorrentInterface* tc,KSharedConfigPtr cfg);
		void setShowListOfFiles(bool on,KSharedConfigPtr cfg);
		void saveState(KSharedConfigPtr cfg);
		void loadState(KSharedConfigPtr cfg);
		void update();
		void filePercentageChanged(bt::TorrentFileInterface* file,float percentage);
		void filePreviewChanged(bt::TorrentFileInterface* file,bool preview);
		
	public slots:
		void onTorrentRemoved(bt::TorrentInterface* tc);

	private slots:
		void showContextMenu(const QPoint & p);
		void onDoubleClicked(const QModelIndex & index);
		void onMissingFileMarkedDND(bt::TorrentInterface* tc);
		
	private:
		void changePriority(bt::Priority newpriority);
		void expandCollapseTree(const QModelIndex& idx, bool expand);
		void expandCollapseSelected(bool expand);
		virtual bool viewportEvent(QEvent *event);
		virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

	private slots:
		void open();
		void downloadFirst();
		void downloadLast();
		void downloadNormal();
		void doNotDownload();
		void deleteFiles();
		void moveFiles();
		void collapseTree();
		void expandTree();

	private:
		bool redraw;
		bt::TorrentInterface* curr_tc;
		TorrentFileModel* model;

		KMenu* context_menu;
		QAction* open_action;
		QAction* download_first_action;
		QAction* download_normal_action;
		QAction* download_last_action;
		QAction* dnd_action;
		QAction* delete_action;
		QAction* move_files_action;
		QAction* collapse_action;
		QAction* expand_action;

		QString preview_path;
		bool show_list_of_files;
		QMap<bt::TorrentInterface*,QByteArray> expanded_state_map;
		QSortFilterProxyModel* proxy_model;
	};

}

#endif
