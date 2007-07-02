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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef FILESELECTDLG_H
#define FILESELECTDLG_H

#include <interfaces/filetreediritem.h>
#include "fileselectdlgbase.h"



namespace kt
{

	class TorrentInterface;
	class FileTreeDirItem;
	class GroupManager;
}

/**
 * @author Joris Guisson
 *
 * Dialog to select which files to download from a multifile torrent.
 */

class FileSelectDlg : public FileSelectDlgBase, public kt::FileTreeRootListener
{
		Q_OBJECT

		kt::TorrentInterface* tc;
		kt::FileTreeDirItem* root;
		
		kt::GroupManager* m_gman;
		
		bool* m_user;
		bool* m_start;

	public:
		FileSelectDlg(kt::GroupManager* gm, bool* user, bool* start, QWidget* parent = 0, const char* name = 0,
					  bool modal = true, WFlags fl = 0);

		virtual ~FileSelectDlg();
		int execute(kt::TorrentInterface* tc);
		
		void loadGroups();
		
		void populateFields();

		void setupMultifileTorrent();
		void setupSinglefileTorrent();

	protected slots:
		virtual void reject();
		virtual void accept();
		void selectAll();
		void selectNone();
		void invertSelection();

	private:
		virtual void treeItemChanged();
	
	private slots:
		void updateSizeLabels();
};

#endif

