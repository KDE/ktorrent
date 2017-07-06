/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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


#ifndef KT_MEDIACONTROLLER_H
#define KT_MEDIACONTROLLER_H

#include <QLabel>
#include <KActionCollection>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>

#include "mediafile.h"
#include "ui_mediacontroller.h"

namespace kt
{

    class MediaPlayer;

    /**
     * Widget containing all the things necessary to control the media playback.
     */
    class MediaController : public QWidget, public Ui_MediaController
    {
        Q_OBJECT
    public:
        MediaController(MediaPlayer* player, KActionCollection* ac, QWidget* parent = 0);
        ~MediaController();


    private slots:
        void playing(const MediaFileRef& file);
        void stopped();
        void metaDataChanged();

    private:
        MediaFileRef current_file;
    };

}

#endif // KT_MEDIACONTROLLER_H
