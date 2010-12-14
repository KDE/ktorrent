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
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <KLineEdit>
#include <KLocale>
#include <util/log.h>
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"



using namespace bt;

namespace kt
{
	

	MediaView::MediaView(MediaModel* model,QWidget* parent)
			: QWidget(parent),model(model)
	{
		filter = new QSortFilterProxyModel(this);
		filter->setSourceModel(model);
		filter->setFilterRole(Qt::DisplayRole);
		filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		search_box = new KLineEdit(this);
		search_box->setClearButtonShown(true);
		search_box->setClickMessage(i18n("Search media files"));
		connect(search_box,SIGNAL(textChanged(QString)),filter,SLOT(setFilterFixedString(QString)));
		layout->addWidget(search_box);
		
		media_tree = new QListView(this);
		media_tree->setModel(filter);
		media_tree->setDragEnabled(true);
		media_tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
		media_tree->setAlternatingRowColors(true);
		layout->addWidget(media_tree);
		
		connect(media_tree,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(onDoubleClicked(QModelIndex)));
	}


	MediaView::~MediaView()
	{
	}
	
	void MediaView::onDoubleClicked(const QModelIndex& index)
	{
		if (!index.isValid())
			return;
		
		QModelIndex idx = filter->mapToSource(index);
		if (!idx.isValid())
			return;
		
		doubleClicked(model->fileForIndex(idx));
	}

}
