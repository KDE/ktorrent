/***************************************************************************
 *   Copyright (C) 2006-2007 by Joris Guisson, Ivan Vasic                  *
 *   joris.guisson@gmail.com                                               *
 *	 ivasic@gmail.com                                                  *
 *									   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include "ui_trackerview.h"
#include <QSortFilterProxyModel>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class TrackerModel;

	/**
	 * @author Ivan Vasic <ivan@ktorrent.org>
	 */
	class TrackerView: public QWidget, public Ui_TrackerView
	{
		Q_OBJECT
	public:
		TrackerView(QWidget *parent);			
		virtual ~TrackerView();
			
		void update();
		void changeTC(bt::TorrentInterface* ti);
		void saveState(KSharedConfigPtr cfg);
		void loadState(KSharedConfigPtr cfg);
			
	public slots:
		virtual void updateClicked();
		virtual void restoreClicked();
		virtual void changeClicked();
		virtual void removeClicked();
		virtual void addClicked();
		virtual void scrapeClicked();
		void currentChanged(const QModelIndex & current,const QModelIndex & previous);
			
	private:
		void torrentChanged(bt::TorrentInterface* ti);
			
	private:
		bt::TorrentInterface* tc;
		TrackerModel* model;
		QSortFilterProxyModel* proxy_model;
	};
}
#endif
