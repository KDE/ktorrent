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
#ifndef PEERVIEW_H
#define PEERVIEW_H

#include <qmap.h>
#include <klistview.h>
#include <qlistview.h>
#include <kpopupmenu.h>
#include <qpoint.h>
#include <util/constants.h>

namespace kt
{
	class PeerInterface;
	class PeerView;
	
	class PeerViewItem : public KListViewItem
	{
		kt::PeerInterface* peer;
		QString m_country;
		bt::Uint32 ip;
		// counter to keep track of how many PeerViewItem objects are in existence
		static bt::Uint32 pvi_count;
	public:
		PeerViewItem(PeerView* pv,kt::PeerInterface* peer);
		virtual ~PeerViewItem();
	
		void update();
		int compare(QListViewItem * i,int col,bool) const;
		kt::PeerInterface* getPeer() { return peer; }
		
	};
	
	/**
	@author Joris Guisson
	*/
	class PeerView : public KListView
	{
		Q_OBJECT
		
		QMap<kt::PeerInterface*,PeerViewItem*> items;
	public:
		PeerView(QWidget *parent = 0, const char *name = 0);
		virtual ~PeerView();
		
	public slots:
		void addPeer(kt::PeerInterface* peer);
		void removePeer(kt::PeerInterface* peer);
		void banPeer(kt::PeerInterface* peer);
		void kickPeer(kt::PeerInterface* peer);
		void update();
		void removeAll();
		void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
		void contextItem(int id);
	private:
		KPopupMenu* menu;
		int ban_id;
		int kick_id;
		PeerViewItem* curr;
	};
}

#endif
