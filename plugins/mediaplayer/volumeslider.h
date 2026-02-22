/*
 *  SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KTVOLUMESLIDER_H
#define KTVOLUMESLIDER_H

#include <QSlider>

class QAudioOutput;

namespace kt
{
class VolumeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget *parent = nullptr);
    explicit VolumeSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setAudioOutput(QAudioOutput *audio);

private:
    QAudioOutput *m_audio = nullptr;
};
}

#endif // KTVOLUMESLIDER_H
