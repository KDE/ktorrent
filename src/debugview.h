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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DEBUGVIEW_H
#define DEBUGVIEW_H

#include <qwidget.h>

class ChunkDownloadView;
class KTorrentMonitor;
class PeerView;

namespace bt
{
	class TorrentControl;
}

/**
@author Joris Guisson
*/
class DebugView : public QWidget
{
	Q_OBJECT
public:
	DebugView(bt::TorrentControl* tor,QWidget *parent = 0, const char *name = 0);
	virtual ~DebugView();
	
	
private:
	ChunkDownloadView* m_cd_view;
	PeerView* m_peer_view;
	KTorrentMonitor* m_mon;
	bt::TorrentControl* m_tor;
};

#endif
