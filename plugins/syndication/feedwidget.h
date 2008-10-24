/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KTFEEDWIDGET_H
#define KTFEEDWIDGET_H

#include <QWidget>
#include "ui_feedwidget.h"

namespace kt
{
	class Feed;
	class FeedWidgetModel;
	class FilterList;

	/**
		@author
	*/
	class FeedWidget : public QWidget,public Ui_FeedWidget
	{
		Q_OBJECT
	public:
		FeedWidget(Feed* feed,FilterList* filters,QWidget* parent);
		virtual ~FeedWidget();

	private slots:
		void downloadClicked();
		void refreshClicked();
		void filtersClicked();
		void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void updated();
		
	signals:
		void downloadLink(const KUrl & link);
		void updateCaption(QWidget* w,const QString & text);
		
	private:
		Feed* feed;
		FeedWidgetModel* model;
		FilterList* filters;
	};

}

#endif
