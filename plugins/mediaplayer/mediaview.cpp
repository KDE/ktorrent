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
#include <QLabel>
#include <QSpacerItem>
#include <ktoolbar.h>
#include <klocale.h>
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
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		layout->addWidget(new QLabel(i18n("Collection:")));
		media_tree = new QTreeView(this);
		media_tree->setModel(model);
		media_tree->header()->hide();
		media_tree->setDragEnabled(true);
		layout->addWidget(media_tree);
		
		connect(media_tree,SIGNAL(doubleClicked(const QModelIndex &)),this,SIGNAL(doubleClicked(const QModelIndex&)));
	}


	MediaView::~MediaView()
	{
	}
	
	QModelIndex MediaView::selectedItem() const
	{
		QModelIndexList rows = media_tree->selectionModel()->selectedRows();
		if (rows.count() > 0)
			return rows.front();
		else
			return QModelIndex();
	}

	

}
