/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTVIDEOWIDGET_H
#define KTVIDEOWIDGET_H

#include <QAction>
#include <QWidget>

class QAction;
class QLabel;
class QSlider;
class QVideoWidget;
class KToolBar;
class KActionCollection;

namespace kt
{
class SeekSlider;
class VideoChunkBar;
class VolumeSlider;
class MediaPlayer;
class MediaFileRef;

/**
 * Widget to display a video
 * @author Joris Guisson
 */
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    VideoWidget(MediaPlayer *player, KActionCollection *ac, QWidget *parent);
    ~VideoWidget() override;

    /**
     * Make the widget full screen or not.
     * @param on
     */
    void setFullScreen(bool on);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *dst, QEvent *event) override;

private Q_SLOTS:
    void play();
    void stop();
    void setControlsVisible(bool on);
    void timerTick(qint64 time);
    void playing(const MediaFileRef &mfile);
    void enableActions(unsigned int flags);

Q_SIGNALS:
    void toggleFullScreen(bool on);

private:
    void inhibitScreenSaver(bool on);
    QString formatTime(qint64 cur, qint64 total);

private:
    QVideoWidget *video;
    MediaPlayer *player;
    SeekSlider *slider;
    KToolBar *tb;
    QAction *play_action;
    QAction *stop_action;
    QLabel *time_label;
    VolumeSlider *volume;
    VideoChunkBar *chunk_bar;
    bool fullscreen;
    quint32 screensaver_cookie;
    quint32 powermanagement_cookie;
};

}

#endif
