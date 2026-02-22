/*
 *  SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "seekslider.h"

#include <QMediaPlayer>

namespace kt
{

SeekSlider::SeekSlider(QWidget *parent)
    : QSlider(parent)
{
}

SeekSlider::SeekSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
}

void SeekSlider::setMediaPlayer(QMediaPlayer *media)
{
    if (m_media == media) {
        return;
    }

    if (m_media) {
        disconnect(this, nullptr, m_media, nullptr);
        disconnect(m_media, nullptr, this, nullptr);
    }

    if (media) {
        setEnabled(media->isSeekable());
        setMaximum(media->duration());
        setSliderPosition(media->position());
        connect(media, &QMediaPlayer::seekableChanged, this, &QSlider::setEnabled);
        connect(media, &QMediaPlayer::durationChanged, this, [this](qint64 duration) -> void {
            setMaximum(duration);
        });
        connect(media, &QMediaPlayer::positionChanged, this, [this](qint64 position) -> void {
            if (!isSliderDown()) {
                setValue(position);
            }
        });
        connect(this, &QSlider::sliderMoved, media, &QMediaPlayer::setPosition);
    }
    m_media = media;
}
}
