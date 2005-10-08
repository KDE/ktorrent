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
#ifndef PEERVIEW_H
#define PEERVIEW_H

#include <qmap.h>
#include <klistview.h>

namespace bt
{
	class Peer;
}

class PeerView;

class PeerViewItem : public KListViewItem
{
	bt::Peer* peer;
public:
	PeerViewItem(PeerView* pv,bt::Peer* peer);

	void update();
	int compare(QListViewItem * i,int col,bool) const;
};

/**
@author Joris Guisson
*/
class PeerView : public KListView
{
	Q_OBJECT
	
	QMap<bt::Peer*,PeerViewItem*> items;
public:
	PeerView(QWidget *parent = 0, const char *name = 0);
	virtual ~PeerView();
	
public slots:
	void addPeer(bt::Peer* peer);
	void removePeer(bt::Peer* peer);
	void update();
	void removeAll();
};

#endif
