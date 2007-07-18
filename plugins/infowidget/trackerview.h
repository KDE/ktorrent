/***************************************************************************
 *   Copyright (C) 2006 by Joris Guisson, Ivan Vasic                       *
 *   joris.guisson@gmail.com                                               *
 *	 ivasic@gmail.com                                                      *
 *																		   *
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
#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include <kurl.h>
#include "trackerviewbase.h"

namespace kt
{
	class TorrentInterface;
	class TorrentFileInterface;
	
	/**
	 * @author Ivan Vasic <ivan@ktorrent.org>
	 */
	class TrackerView: public TrackerViewBase
	{
			Q_OBJECT
		public:
			TrackerView(QWidget *parent = 0, const char *name = 0);
			virtual ~TrackerView();
			
			void update();
			void changeTC(TorrentInterface* ti);
			
		public slots:
			virtual void btnUpdate_clicked();
			virtual void btnRestore_clicked();
			virtual void btnChange_clicked();
			virtual void btnRemove_clicked();
			virtual void btnAdd_clicked();
    		virtual void listTrackers_currentChanged(QListViewItem*);
			void onLoadingFinished(const KURL & ,bool,bool);
			
		private:
			void torrentChanged(TorrentInterface* ti);
			
		private:
			TorrentInterface* tc;
	
	};
}
#endif
