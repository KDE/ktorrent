/*
 *  SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KTSEEKSLIDER_H
#define KTSEEKSLIDER_H

#include <QSlider>

class QMediaPlayer;

namespace kt
{
class SeekSlider : public QSlider
{
    Q_OBJECT

public:
    explicit SeekSlider(QWidget *parent = nullptr);
    explicit SeekSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setMediaPlayer(QMediaPlayer *media);

private:
    QMediaPlayer *m_media = nullptr;
};
}

#endif // KTSEEKSLIDER_H
