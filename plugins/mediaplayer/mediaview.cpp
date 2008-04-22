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
#include <ktoolbar.h>
#include <klocale.h>
#include <util/log.h>
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"

using namespace bt;

namespace kt
{

	MediaView::MediaView(MediaPlayer* player,MediaModel* model,QWidget* parent)
			: QWidget(parent),player(player),model(model),cnt(0)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		tool_bar = new KToolBar(this);
		tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(tool_bar);
		
		play_slider = new Phonon::SeekSlider(this);
		play_slider->setMediaObject(player->media0bject());
		layout->addWidget(play_slider);
		
		info_label = new QLabel(this);
		info_label->setMargin(5);
		info_label->setFrameShadow(QFrame::Sunken);
		info_label->setFrameShape(QFrame::StyledPanel);
		info_label->setBackgroundRole(QPalette::Base);
		info_label->setAutoFillBackground(true);
		info_label->setWordWrap(true);
		layout->addWidget(info_label);
		info_label->setText(i18n("Ready to play"));
		
		media_tree = new QTreeView(this);
		media_tree->setModel(model);
		media_tree->header()->hide();
		layout->addWidget(media_tree);
		
		volume = new Phonon::VolumeSlider(this);
		volume->setAudioOutput(player->output());
		layout->addWidget(volume);
		
		connect(media_tree->selectionModel(),SIGNAL(selectionChanged(const QItemSelection & , const QItemSelection & )),
				this,SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
		connect(media_tree,SIGNAL(doubleClicked(const QModelIndex &)),this,SIGNAL(doubleClicked(const QModelIndex&)));
		connect(player,SIGNAL(stopped()),this,SLOT(stopped()));
		connect(player->media0bject(),SIGNAL(metaDataChanged()),this,SLOT(metaDataChanged()));
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

	void MediaView::playing(const QModelIndex & index)
	{
		if (!index.isValid())
			stopped();
		else
		{
			cnt++;
			current_file = model->data(index,Qt::DisplayRole).toString();
			info_label->setText(i18n("Playing: <b>%1</b>",current_file));
		}
	}
	
	void MediaView::stopped()
	{
		if (cnt > 0)
			cnt--;
		
		if (cnt == 0)
		{
			info_label->setText(i18n("Ready to play"));
			current_file = QString();
		}
	}
	
	void MediaView::metaDataChanged()
	{
		QString extra_data;
		QStringList artist = player->media0bject()->metaData(Phonon::ArtistMetaData);
		QStringList title = player->media0bject()->metaData(Phonon::TitleMetaData);
		QStringList album = player->media0bject()->metaData(Phonon::AlbumMetaData);
		
		bool has_artist = artist.count() > 0 && artist[0].length() > 0;
		bool has_title = title.count() > 0 && title[0].length() > 0;
		bool has_album = album.count() > 0 && album[0].length() > 0;
		
		if (has_artist && has_title && has_album)
		{
			extra_data = i18n("Title: <b>%1</b><br/>Artist: <b>%2</b><br/>Album: <b>%3</b>",title[0],artist[0],album[0]);
		}
		else if (has_title && has_artist)
		{
			extra_data = i18n("Title: <b>%1</b><br/>Artist: <b>%2</b>",title[0],artist[0]);
		}
		else if (has_title)
		{
			extra_data = i18n("Title: <b>%1</b>",title[0]);
		}
		else
			return;
		
		if (cnt > 0)
		{
			info_label->setText(i18n("Playing: <b>%1</b><br/>\n%2",current_file,extra_data));
		}
	}

}
