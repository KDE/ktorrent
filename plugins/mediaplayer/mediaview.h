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
#ifndef KTMEDIAVIEW_H
#define KTMEDIAVIEW_H


#include <QListView>
#include <QSortFilterProxyModel>
#include "mediafile.h"

class KLineEdit;

namespace kt
{
	class MediaModel;

	/**
		@author
	*/
	class MediaView : public QWidget
	{
		Q_OBJECT
	public:
		MediaView(MediaModel* model,QWidget* parent);
		virtual ~MediaView();
		
	signals:
		void doubleClicked(const MediaFileRef & mf);
		
	private slots:
		void onDoubleClicked(const QModelIndex & index);

	private:
		MediaModel* model;
		QListView* media_tree;
		KLineEdit* search_box;
		QSortFilterProxyModel* filter;
	};

}

#endif
