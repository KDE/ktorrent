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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef IWFILETREEDIRITEM_H
#define IWFILETREEDIRITEM_H

#include <klistview.h>
#include <libutil/constants.h>
#include <libutil/ptrmap.h>

class IWFileTreeItem;

using bt::Uint32;

namespace bt
{
	class TorrentFile;
	class TorrentControl;
}


/**
 * @author Joris Guisson
 *
 * Directory item in the InfoWidget's file view.
 */
class IWFileTreeDirItem : public QCheckListItem
{
	QString name;
	Uint32 size;
	bt::PtrMap<QString,IWFileTreeItem> children;
	bt::PtrMap<QString,IWFileTreeDirItem> subdirs;
	IWFileTreeDirItem* parent;
	bool manual_change;
public:
	IWFileTreeDirItem(KListView* klv,const QString & name);
	IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name);
	virtual ~IWFileTreeDirItem();

	void insert(const QString & path,bt::TorrentFile & file);

	/**
	 * Recursivly walk the tree to find the TorrentFile which
	 * is shown by a QListViewItem (which should be an IWFileTreeItem).
	 * If item can't be found or item is an IWFileTreeDirItem, a reference to
	 * TorrentFile::null will be returned. In which case the isNull() function
	 * of TorrentFile will return true
	 * @param item Pointer to the QListViewItem
	 * @return A reference to the TorrentFile
	 */
	bt::TorrentFile & findTorrentFile(QListViewItem* item);

	/**
	 * Set all items checked or not.
	 * @param on true everything checked, false everything not checked
	 */
	void setAllChecked(bool on);

	/**
	 * Invert all items, checked items become unchecked and unchecked become checked.
	 */
	void invertChecked();

	/**
	 * Update the preview information.
	 * @param tc The TorrentControl object
	 */
	void updatePreviewInformation(bt::TorrentControl* tc);

	/**
	 * Called by the child to notify the parent it's state has changed.
	 */
	void childStateChange();

	
private:
	virtual void stateChange(bool on);
	virtual int compare(QListViewItem* i, int col, bool ascending) const;
	bool allChildrenOn();
};

#endif
