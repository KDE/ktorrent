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
#ifndef BTCHUNKDOWNLOADVIEW_H
#define BTCHUNKDOWNLOADVIEW_H

#include <klistview.h>
#include <qtimer.h>
#include <qmap.h>

namespace bt
{
	class ChunkDownload;
}


/**
@author Joris Guisson
*/
class ChunkDownloadView : public KListView
{
	Q_OBJECT
	QTimer timer;
	QMap<bt::ChunkDownload*,KListViewItem*> items;
public:
	ChunkDownloadView(QWidget *parent = 0, const char *name = 0);
	virtual ~ChunkDownloadView();

public slots:
	void addDownload(bt::ChunkDownload* cd);
	void removeDownload(bt::ChunkDownload* cd);
	void removeAll();
	void update();
	
private:
	void update(const bt::ChunkDownload* cd,KListViewItem* it);
};



#endif
