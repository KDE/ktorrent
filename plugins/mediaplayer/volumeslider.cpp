/*
 *  SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "volumeslider.h"

#include <QAudioOutput>

namespace
{
// volume slider is logarithmic from 0 to 100
// audio output is linear from 0 to 1

float SliderToAudio(const int volume)
{
    return QtAudio::convertVolume(static_cast<float>(volume) / 100, QtAudio::LogarithmicVolumeScale, QtAudio::LinearVolumeScale);
};

int AudioToSlider(const float volume)
{
    return static_cast<int>(QtAudio::convertVolume(volume, QtAudio::LinearVolumeScale, QtAudio::LogarithmicVolumeScale) * 100);
};
}

namespace kt
{

VolumeSlider::VolumeSlider(QWidget *parent)
    : QSlider(parent)
{
    setRange(0, 100);
}

VolumeSlider::VolumeSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
    setRange(0, 100);
}

void VolumeSlider::setAudioOutput(QAudioOutput *audio)
{
    if (m_audio == audio) {
        return;
    }

    if (m_audio) {
        disconnect(this, nullptr, m_audio, nullptr);
        disconnect(m_audio, nullptr, this, nullptr);
    }

    if (audio) {
        setValue(AudioToSlider(audio->volume()));
        connect(this, &QSlider::sliderMoved, audio, [audio](const int volume_percent) {
            audio->setVolume(SliderToAudio(volume_percent));
        });
        connect(audio, &QAudioOutput::volumeChanged, this, [this](const float volume_fraction) {
            if (!isSliderDown()) {
                setValue(AudioToSlider(volume_fraction));
            }
        });
    }
    m_audio = audio;
}
}
