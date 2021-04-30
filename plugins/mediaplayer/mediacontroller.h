/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_MEDIACONTROLLER_H
#define KT_MEDIACONTROLLER_H

#include <KActionCollection>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <QLabel>

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
    MediaController(MediaPlayer *player, KActionCollection *ac, QWidget *parent = 0);
    ~MediaController() override;

private Q_SLOTS:
    void playing(const MediaFileRef &file);
    void stopped();
    void metaDataChanged();

private:
    MediaFileRef current_file;
};

}

#endif // KT_MEDIACONTROLLER_H
