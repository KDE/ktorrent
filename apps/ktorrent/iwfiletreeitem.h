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
#ifndef IWFILETREEITEM_H
#define IWFILETREEITEM_H

#include <klistview.h>
#include <util/constants.h>

class IWFileTreeDirItem;

using bt::Uint32;

namespace bt
{
	class TorrentFile;
	class TorrentInterface;
}

/**
 * @author Joris Guisson
 *
 * File item in the InfoWidget's file view.
*/
class IWFileTreeItem : public QCheckListItem
{
	QString name;
	Uint32 size;
	kt::TorrentFileInterface & file;
	IWFileTreeDirItem* parent;
	bool manual_change;
public:
	IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,kt::TorrentFileInterface & file);
	virtual ~IWFileTreeItem();

	kt::TorrentFileInterface & getTorrentFile() {return file;}
	void updatePreviewInformation(kt::TorrentInterface* tc);
	void setChecked(bool on);
	
private:
	void init();
	virtual void stateChange(bool on);
	virtual int compare(QListViewItem* i, int col, bool ascending) const;
};




#endif
