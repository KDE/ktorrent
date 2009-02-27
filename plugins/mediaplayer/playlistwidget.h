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

#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTreeView>
#include <QToolBar>
#include <QComboBox>
#include <QCheckBox>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <KSharedConfig>


namespace kt
{
	class PlayList;
	class MediaPlayer;
	
	class PlayListWidget : public QWidget
	{
		Q_OBJECT
	public:
		PlayListWidget(MediaPlayer* player,QWidget* parent);
		virtual ~PlayListWidget();
		
		/// Get the media tool bar
		QToolBar* mediaToolBar() {return tool_bar;}
		
		/// Get the play list
		PlayList* playList() {return play_list;}
		
		/// Get the current selected item
		QModelIndex selectedItem() const;
		
		void saveState(KSharedConfigPtr cfg);
		void loadState(KSharedConfigPtr cfg);
		
	public slots:
		QModelIndex play();
		void playing(const QString & file);
		void stopped();
		
	private slots:
		void modeActivated(int idx);
		void metaDataChanged();
		void onSelectionChanged(const QItemSelection & s, const QItemSelection & d);
		void doubleClicked(const QModelIndex & index);
		
	signals:
		void randomModeActivated();
		void selectionChanged(const QModelIndex & idx);
		void doubleClicked(const QString & file);
		
	private:
		MediaPlayer* player;
		QTreeView* play_list_view;
		PlayList* play_list;
		QToolBar* tool_bar;
		Phonon::VolumeSlider* volume;
		Phonon::SeekSlider* play_slider;
		QComboBox* queue_mode;
		QLabel* info_label;
		unsigned int cnt;
		QString current_file;
	};
}

#endif // PLAYLISTWIDGET_H
