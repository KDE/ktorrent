/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
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
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
***************************************************************************/

#include "playlistwidget.h"

#include <QVBoxLayout>
#include <ktoolbar.h>
#include <klocale.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"
#include "playlist.h"
#include <QFile>


namespace kt
{
	PlayListWidget::PlayListWidget(MediaPlayer* player,QWidget* parent) : QWidget(parent),player(player),cnt(0)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		play_list_view = new QTreeView(this);
		play_list = new PlayList(this);
		play_list_view->setModel(play_list);
		play_list_view->setDragEnabled(true);
		play_list_view->setDropIndicatorShown(true);
		play_list_view->setAcceptDrops(true);
		play_list_view->setAlternatingRowColors(true);
		play_list_view->setRootIsDecorated(false);
		layout->addWidget(play_list_view);
		
		QHBoxLayout* hbox = new QHBoxLayout(0);
		layout->addLayout(hbox);
		
		tool_bar = new KToolBar(this);
		tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		hbox->addWidget(tool_bar);
		
		play_slider = new Phonon::SeekSlider(this);
		play_slider->setMediaObject(player->media0bject());
		hbox->addWidget(play_slider);
		
		info_label = new QLabel(this);
		info_label->setMargin(5);
		info_label->setFrameShadow(QFrame::Sunken);
		info_label->setFrameShape(QFrame::StyledPanel);
		info_label->setBackgroundRole(QPalette::Base);
		info_label->setAutoFillBackground(true);
		info_label->setWordWrap(true);
		layout->addWidget(info_label);
		info_label->setText(i18n("Ready to play"));
		
		QHBoxLayout* hlayout = new QHBoxLayout(0);
		hlayout->addWidget(new QLabel(i18n("Mode:"),this));
		
		queue_mode = new QComboBox(this);
		queue_mode->addItem(i18n("Single File"));
		queue_mode->addItem(i18n("All Files"));
		queue_mode->addItem(i18n("Random Files"));
		queue_mode->setCurrentIndex(MediaPlayerPluginSettings::playMode());
		hlayout->addWidget(queue_mode);
		hbox->addLayout(hlayout);
		
		QSpacerItem* s = new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum);
		hlayout->addItem(s);
		
		skip_incomplete = new QCheckBox(i18n("Skip incomplete files"),this);
		skip_incomplete->setChecked(MediaPlayerPluginSettings::skipIncomplete());
		hbox->addWidget(skip_incomplete);
		
		volume = new Phonon::VolumeSlider(this);
		volume->setAudioOutput(player->output());
		hbox->addWidget(volume);
		
		
		connect(player,SIGNAL(stopped()),this,SLOT(stopped()));
		connect(player,SIGNAL(playing(QString)),this,SLOT(playing(QString)));
		connect(skip_incomplete,SIGNAL(toggled(bool)),this,SLOT(skipIncompleteChecked(bool)));
		connect(queue_mode,SIGNAL(activated(int)),this,SLOT(modeActivated(int)));
	}
	
	PlayListWidget::~PlayListWidget() 
	{
	}

	void PlayListWidget::playing(const QString & file)
	{
		if (file.isEmpty())
		{
			stopped();
		}
		else
		{
			cnt++;
			current_file = file;
			metaDataChanged();
		}
	}

	void PlayListWidget::stopped()
	{
		if (cnt > 0)
			cnt--;
		
		if (cnt == 0)
		{
			info_label->setText(i18n("Ready to play"));
			current_file = QString();
		}
	}

	void PlayListWidget::metaDataChanged()
	{
		QString extra_data;
		QByteArray encoded = QFile::encodeName(current_file);
		TagLib::FileRef ref(encoded,true,TagLib::AudioProperties::Fast);
		if (ref.isNull())
			return;
		
		TagLib::Tag* tag = ref.tag();
		if (!tag)
			return;
		
		QString artist = tag->artist().toCString(true);
		QString title =  tag->title().toCString(true);
		QString album = tag->album().toCString(true);
		
		bool has_artist = !artist.isEmpty();
		bool has_title = !title.count();
		bool has_album = !album.count();
		
		if (has_artist && has_title && has_album)
		{
			extra_data = i18n("Title: <b>%1</b><br/>Artist: <b>%2</b><br/>Album: <b>%3</b>",title,artist,album);
			info_label->setText(i18n("Playing: <b>%1</b><br/>\n%2",current_file,extra_data));
		}
		else if (has_title && has_artist)
		{
			extra_data = i18n("Title: <b>%1</b><br/>Artist: <b>%2</b>",title,artist);
			info_label->setText(i18n("Playing: <b>%1</b><br/>\n%2",current_file,extra_data));
		}
		else if (has_title)
		{
			extra_data = i18n("Title: <b>%1</b>",title);
			info_label->setText(i18n("Playing: <b>%1</b><br/>\n%2",current_file,extra_data));
		}
		else
		{
			info_label->setText(i18n("Playing: <b>%1</b>",current_file));
		}
	}

	void PlayListWidget::skipIncompleteChecked(bool on)
	{
		MediaPlayerPluginSettings::setSkipIncomplete(on);
		MediaPlayerPluginSettings::self()->writeConfig();
	}

	void PlayListWidget::modeActivated(int idx)
	{
		MediaPlayerPluginSettings::setPlayMode(idx);
		MediaPlayerPluginSettings::self()->writeConfig();
		if (idx == 2)
			randomModeActivated();
	}

}