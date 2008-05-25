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

#ifndef UPNPPREFWIDGET_H
#define UPNPPREFWIDGET_H

#include <qmap.h>
#include "upnprouter.h"
#include "upnpwidget.h"

class KListViewItem;

namespace bt
{
	class WaitJob;
}

namespace kt
{
	
	/**
	 * Widget for the UPnP pref dialog page.
	 */
	class UPnPPrefWidget : public UPnPWidget,public net::PortListener
	{
		Q_OBJECT
	
	public:
		UPnPPrefWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
		virtual ~UPnPPrefWidget();
		
		void shutdown(bt::WaitJob* job);
		
	
	public slots:
		/**
		 * Add a device to the list. 
		 * @param r The device
		 */
		void addDevice(UPnPRouter* r);
	
	signals:
		/**
		 * Emitted when the user presses the rescan button.
		 */
		void rescan();
		
	
	protected slots:
		void onForwardBtnClicked();
		void onUndoForwardBtnClicked();
		void onRescanClicked();
		void updatePortMappings();
		
	private:
		virtual void portAdded(const net::Port & port);
		virtual void portRemoved(const net::Port & port);
		
	private:
		QMap<KListViewItem*,UPnPRouter*> itemmap;
		UPnPRouter* def_router;
	};
}

#endif

