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
#ifndef KTFILETREEDIRITEM_H
#define KTFILETREEDIRITEM_H

#include <klistview.h>
#include <util/constants.h>
#include <util/ptrmap.h>

namespace kt
{
	using namespace bt;
	
	class FileTreeItem;
	class TorrentFileInterface;
	class TorrentInterface;
	
	class FileTreeRootListener 
	{
	public: 
		/// An item in the file tree has changed his state
		virtual void treeItemChanged() = 0;
	};

	/**
	 * @author Joris Guisson
	 *
	 * Directory item the file tree showing the files in a multifile torrent
	 */
	class FileTreeDirItem : public QCheckListItem
	{
	protected:
		QString name;
		Uint64 size;
		bt::PtrMap<QString,FileTreeItem> children;
		bt::PtrMap<QString,FileTreeDirItem> subdirs;
		FileTreeDirItem* parent;
		bool manual_change;
		FileTreeRootListener* root_listener;
	public:
		FileTreeDirItem(KListView* klv,const QString & name,FileTreeRootListener* rl = 0);
		FileTreeDirItem(FileTreeDirItem* parent,const QString & name);
		virtual ~FileTreeDirItem();
		
		/// Get the path of the directory (if this is the root directory / will be returned)
		QString getPath() const;

		/**
		 * Recursively insert a TorrentFileInterface.
		 * @param path Path of file
		 * @param file File itself
		 */
		void insert(const QString & path,kt::TorrentFileInterface & file);

		/**
		 * Recursivly walk the tree to find the TorrentFile which
		 * is shown by a QListViewItem (which should be an FileTreeItem).
		 * If item can't be found or item is an FileTreeDirItem, a reference to
		 * TorrentFile::null will be returned. In which case the isNull() function
		 * of TorrentFile will return true
		 * @param item Pointer to the QListViewItem
		 * @return A reference to the TorrentFile
		 */
		kt::TorrentFileInterface & findTorrentFile(QListViewItem* item);

		/**
		 * Set all items checked or not.
		 * @param on true everything checked, false everything not checked
		 * @param keep_data In case of unchecking keep the data or not
		 */
		void setAllChecked(bool on,bool keep_data = false);

		/**
		 * Invert all items, checked items become unchecked and unchecked become checked.
		 */
		void invertChecked();

		/**
		 * Called by the child to notify the parent it's state has changed.
		 */
		void childStateChange();

		FileTreeDirItem* getParent() {return parent;}
		
		/// Recusively get the total number of bytes to download
		Uint64 bytesToDownload() const;

	protected:
		/**
		 * Can be overrided by subclasses, so they can use their own
		 * custom FileTreeItem's. Will be called in insert.
		 * @param name Name of the file
		 * @param file The TorrentFileInterface
		 * @return A newly created FileTreeItem
		 */
		virtual FileTreeItem* newFileTreeItem(const QString & name,
											  TorrentFileInterface & file);


		/**
		 * Can be overrided by subclasses, so they can use their own
		 * custom FileTreeDirItem's. Will be called in insert.
		 * @param subdir The name of the subdir
		 * @return A newly created FileTreeDirItem
		 */
		virtual FileTreeDirItem* newFileTreeDirItem(const QString & subdir);
		
		
		/**
		 * Subclasses should override this if they want to show a confirmation dialog. 
		 * @return What to do (i.e. keep the data, get rid of it or do nothing
		 */
		virtual bt::ConfirmationResult confirmationDialog();
		
	private:
		virtual void stateChange(bool on);
		virtual int compare(QListViewItem* i, int col, bool ascending) const;
		bool allChildrenOn();
	};

}

#endif
