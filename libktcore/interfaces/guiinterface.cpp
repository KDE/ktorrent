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
#include <interfaces/torrentinterface.h>
#include "guiinterface.h"


namespace kt
{

	GUIInterface::GUIInterface()
	{}


	GUIInterface::~GUIInterface()
	{}


	void GUIInterface::notifyViewListeners(bt::TorrentInterface* tc)
	{	
		foreach (ViewListener* vl,listeners)
			vl->currentTorrentChanged(tc);
	}
	
	void GUIInterface::addViewListener(ViewListener* vl)
	{
		listeners.append(vl);
	}

	void GUIInterface::removeViewListener(ViewListener* vl)
	{
		listeners.removeAll(vl);
	}
	
	void GUIInterface::addCurrentTabPageListener(CurrentTabPageListener* ctpl)
	{
		ctp_listeners.append(ctpl);
	}

	void GUIInterface::removeCurrentTabPageListener(CurrentTabPageListener* ctpl)
	{
		ctp_listeners.removeAll(ctpl);
	}
	
	void GUIInterface::notifyCurrentTabPageListeners(QWidget* page)
	{
		foreach (CurrentTabPageListener* ctpl,ctp_listeners)
			ctpl->currentTabPageChanged(page);
	}
}
