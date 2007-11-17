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

#include <qtreewidget.h>
#include <util/constants.h>
#include <util/ptrmap.h>
#include <ktorrent_export.h>
#include "filetreeitem.h"

namespace bt
{
	class TorrentFileInterface;
}

namespace kt
{	
		
	class FileTreeRootListener 
	{
	public: 
		/// An item in the file tree has changed his state
		virtual void treeItemChanged() = 0;
		virtual ~FileTreeRootListener() {}
	};

	/**
	 * @author Joris Guisson
	 *
	 * Directory item the file tree showing the files in a multifile torrent
	 */
	class KTORRENT_EXPORT FileTreeDirItem : public QTreeWidgetItem
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
		FileTreeDirItem(QTreeWidget* klv,const QString & name,FileTreeRootListener* rl = 0);
		FileTreeDirItem(FileTreeDirItem* parent,const QString & name);
		virtual ~FileTreeDirItem();

		/// Get the path of the directory (if this is the root directory / will be returned)
		QString getPath() const;

		/**
		 * Recursively insert a TorrentFileInterface.
		 * @param path Path of file
		 * @param file File itself
		 * @param options What to do when the file is deselected
		 */
		void insert(const QString & path,bt::TorrentFileInterface & file,DeselectOptions options);

		/**
		 * Recursivly walk the tree to find the TorrentFile which
		 * is shown by a QListViewItem (which should be an FileTreeItem).
		 * If item can't be found or item is an FileTreeDirItem, a reference to
		 * TorrentFile::null will be returned. In which case the isNull() function
		 * of TorrentFile will return true
		 * @param item Pointer to the QListViewItem
		 * @return A reference to the TorrentFile
		 */
		bt::TorrentFileInterface & findTorrentFile(QTreeWidgetItem* item);

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

		/// Is the item checked
		bool isOn() const {return checkState(0) == Qt::Checked;}

	protected:
		/**
		 * Can be overrided by subclasses, so they can use their own
		 * custom FileTreeItem's. Will be called in insert.
		 * @param name Name of the file
		 * @param file The TorrentFileInterface
		 * @param options What to when a file gets deselected by the user
		 * @return A newly created FileTreeItem
		 */
		virtual FileTreeItem* newFileTreeItem(const QString & name,bt::TorrentFileInterface & file,DeselectOptions options);


		/**
		 * Can be overrided by subclasses, so they can use their own
		 * custom FileTreeDirItem's. Will be called in insert.
		 * @param subdir The name of the subdir
		 * @return A newly created FileTreeDirItem
		 */
		virtual FileTreeDirItem* newFileTreeDirItem(const QString & subdir);
	private:
		void stateChange(bool on);
		virtual void setData(int column, int role, const QVariant& value); // reimplemented to detect user checking and unchecking the item
//		virtual int compare(QListViewItem* i, int col, bool ascending) const;
		bool allChildrenOn();
	};

}

#endif
