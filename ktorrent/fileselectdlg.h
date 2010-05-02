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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef FILESELECTDLG_H
#define FILESELECTDLG_H

#include <KDialog>
#include "ui_fileselectdlg.h"
#include <QSortFilterProxyModel>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class GroupManager;
	class TorrentFileModel;
	class Group;

	/**
	 * @author Joris Guisson
	 *
	 * Dialog to select which files to download from a multifile torrent.
	 */
	class FileSelectDlg : public KDialog,public Ui_FileSelectDlg
	{
		Q_OBJECT

		bt::TorrentInterface* tc;
		TorrentFileModel* model;
		kt::GroupManager* gman;
		bool* start;
		bool* skip_check;
		QList<int> encodings;
		kt::Group* initial_group;
		bool show_file_tree;
		QSortFilterProxyModel* filter_model;
	public:
		FileSelectDlg(kt::GroupManager* gman,const QString & group_hint,QWidget* parent);
		virtual ~FileSelectDlg();
		
		int execute(bt::TorrentInterface* tc, bool* start,bool* skip_check,const QString & location_hint);
		
		/**
		 * Load the state of the dialog
		 */
		void loadState(KSharedConfigPtr cfg);
		
		/**
		 * Save the state of the dialog
		 */
		void saveState(KSharedConfigPtr cfg);
		
	protected slots:
		virtual void reject();
		virtual void accept();
		void selectAll();
		void selectNone();
		void invertSelection();
		void updateSizeLabels();
		void onCodecChanged(const QString & text);
		void groupActivated(int idx);
		void fileTree(bool on);
		void fileList(bool on);
		void setShowFileTree(bool on);
		void setFilter(const QString & filter);
		void updateExistingFiles();

	private:
		void populateFields(const QString & location_hint);
		void loadGroups();
	};
}

#endif

