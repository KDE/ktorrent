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
#include "mediaview.h"
#include "mediamodel.h"
#include "audioplayer.h"

namespace kt
{

	MediaView::MediaView(AudioPlayer* player,MediaModel* model,QWidget* parent)
			: QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		tool_bar = new QToolBar(this);
		layout->addWidget(tool_bar);
		
		play_slider = new QSlider(Qt::Horizontal,this);
		layout->addWidget(play_slider);
		
		media_tree = new QTreeView(this);
		media_tree->setModel(model);
		media_tree->header()->hide();
		layout->addWidget(media_tree);
		
		volume = new Phonon::VolumeSlider(this);
		volume->setAudioOutput(player->output());
		layout->addWidget(volume);
		
		connect(media_tree->selectionModel(),SIGNAL(selectionChanged(const QItemSelection & , const QItemSelection & )),
				this,SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
		
		player->setSlider(play_slider);
	}


	MediaView::~MediaView()
	{
	}
	
	void MediaView::onSelectionChanged(const QItemSelection & s, const QItemSelection & d)
	{
		Q_UNUSED(d);
		QModelIndexList idx = s.indexes();
		if (idx.count() > 0)
			selectionChanged(idx.front());
		else
			selectionChanged(QModelIndex());
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
