/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KTQUEUEMANAGERWIDGET_H
#define KTQUEUEMANAGERWIDGET_H

#include <QWidget>
#include <ksharedconfig.h>
#include "ui_queuemanagerwidget.h"

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class QueueManager;
	class QueueManagerModel;
	
	/**
	 * @author Joris Guisson
	 * 
	 * Widget for the QueueManager
	*/
	class QueueManagerWidget : public QWidget, public Ui_QueueManagerWidget
	{
		Q_OBJECT
	public:
		QueueManagerWidget(QueueManager* qman,QWidget* parent);
		virtual ~QueueManagerWidget();
		
		/// Save the widget state
		void saveState(KSharedConfigPtr cfg);
		/// Load the widget state
		void loadState(KSharedConfigPtr cfg);
		
	public slots:
		void onTorrentAdded(bt::TorrentInterface* tc);
		void onTorrentRemoved(bt::TorrentInterface* tc);
		
	private slots:
		void moveDownloadUpClicked();
		void moveDownloadDownClicked();
		void moveUploadUpClicked();
		void moveUploadDownClicked();

	private:
		QueueManagerModel* uploads;
		QueueManagerModel* downloads;
		QueueManager* qman;
	};
}

#endif
