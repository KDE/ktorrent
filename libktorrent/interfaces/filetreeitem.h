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
#ifndef KTFILETREEITEM_H
#define KTFILETREEITEM_H

#include <klistview.h>
#include <util/constants.h>

using namespace bt;

namespace kt
{
	class TorrentFileInterface;
	class TorrentInterface;
	class FileTreeDirItem;
	
	

	/**
	 * @author Joris Guisson
	 *
	 * File item part of a tree which shows the files in a multifile torrent.
	 * This is derived from QCheckListItem, if the user checks or unchecks the box,
	 * wether or not to download a file will be changed.
	 */
	class FileTreeItem : public QCheckListItem
	{
	protected:
		QString name;
		TorrentFileInterface & file;
		FileTreeDirItem* parent;
		bool manual_change;
	public:
		/**
		 * Constructor, set the parent, name and file
		 * @param item Parent item
		 * @param name Name of file
		 * @param file THe TorrentFileInterface
		 * @return 
		 */
		FileTreeItem(FileTreeDirItem* item,const QString & name,TorrentFileInterface & file);
		virtual ~FileTreeItem();

		/// Get a reference to the TorrentFileInterface
		TorrentFileInterface & getTorrentFile() {return file;}
			
		/**
		 * Set the box checked or not.
		 * @param on Checked or not
		 * @param keep_data In case of unchecking keep the data or not
		 */
		void setChecked(bool on,bool keep_data = false);
		
		/// Get the number of bytes to download in this file
		Uint64 bytesToDownload() const;
		
	
	private:
		void init();
		virtual void stateChange(bool on);
		void updatePriorityText();
		
	protected:
		virtual int compare(QListViewItem* i, int col, bool ascending) const;
		
		/**
		 * Subclasses should override this if they want to show a confirmation dialog. 
		 * @return What to do (i.e. keep the data, get rid of it or do nothing
		 */
		virtual bt::ConfirmationResult confirmationDialog();
	};
}

#endif
