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
#include <qsplitter.h> 
#include <libtorrent/torrentcontrol.h>
#include <qlayout.h>
#include "debugview.h"
#include "chunkdownloadview.h"
#include "peerview.h"
#include "ktorrentmonitor.h"

DebugView::DebugView(bt::TorrentControl* tor,QWidget *parent, const char *name)
	: QWidget(parent, name),m_tor(tor)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setAutoAdd(true);
	
	QSplitter* s = new QSplitter(this);
	m_cd_view = new ChunkDownloadView(s);
	m_peer_view = new PeerView(s);
	m_mon = new KTorrentMonitor(tor,m_peer_view,m_cd_view);
	s->moveToFirst(m_cd_view);
	s->moveToLast(m_peer_view);
	s->setOrientation(Qt::Horizontal);
	connect(tor,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	setCaption(tor->getTorrentName());
}


DebugView::~DebugView()
{
	delete m_mon;
}


#include "debugview.moc"
