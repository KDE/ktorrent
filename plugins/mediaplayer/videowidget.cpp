/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "videowidget.h"

#include <QAction>
#include <QHBoxLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KLocalizedString>
#include <KToggleFullScreenAction>
#include <KToolBar>

#include <phonon/AudioOutput>
#include <phonon/Global>
#include <phonon/Path>
#include <phonon/SeekSlider>
#include <phonon/VolumeSlider>

#include "mediaplayer.h"
#include "powermanagementinhibit_interface.h"
#include "screensaver_interface.h"
#include "videochunkbar.h"
#include <torrent/chunkbar.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
VideoWidget::VideoWidget(MediaPlayer *player, KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
    , player(player)
    , chunk_bar(nullptr)
    , fullscreen(false)
    , screensaver_cookie(0)
    , powermanagement_cookie(0)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);

    video = new Phonon::VideoWidget(this);
    Phonon::createPath(player->media0bject(), video);
    video->installEventFilter(this);

    chunk_bar = new VideoChunkBar(player->getCurrentSource(), this);
    chunk_bar->setVisible(player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream);

    QHBoxLayout *hlayout = new QHBoxLayout(nullptr);

    play_action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), i18n("Play"), this);
    connect(play_action, &QAction::triggered, this, &VideoWidget::play);

    stop_action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-stop")), i18n("Stop"), this);
    connect(stop_action, &QAction::triggered, this, &VideoWidget::stop);

    tb = new KToolBar(this);
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tb->addAction(play_action);
    tb->addAction(ac->action(QStringLiteral("media_pause")));
    tb->addAction(stop_action);
    QAction *tfs = ac->action(QStringLiteral("video_fullscreen"));
    connect(tfs, &QAction::toggled, this, &VideoWidget::toggleFullScreen);
    tb->addAction(tfs);

    slider = new Phonon::SeekSlider(this);
    slider->setMediaObject(player->media0bject());
    slider->setMaximumHeight(tb->iconSize().height());

    volume = new Phonon::VolumeSlider(this);
    volume->setAudioOutput(player->output());
    volume->setMaximumHeight(tb->iconSize().height());
    volume->setMaximumWidth(5 * tb->iconSize().width());

    time_label = new QLabel(this);
    time_label->setText(formatTime(player->media0bject()->currentTime(), player->media0bject()->totalTime()));
    time_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    hlayout->addWidget(tb);
    hlayout->addWidget(slider);
    hlayout->addWidget(volume);
    hlayout->addWidget(time_label);

    chunk_bar->setFixedHeight(hlayout->sizeHint().height() * 0.75);

    vlayout->addWidget(chunk_bar);
    vlayout->addWidget(video);
    vlayout->addLayout(hlayout);

    connect(player->media0bject(), &Phonon::MediaObject::tick, this, &VideoWidget::timerTick);
    connect(player, &MediaPlayer::playing, this, &VideoWidget::playing);
    connect(player, &MediaPlayer::enableActions, this, &VideoWidget::enableActions);

    inhibitScreenSaver(true);
}

VideoWidget::~VideoWidget()
{
    inhibitScreenSaver(false);
}

void VideoWidget::play()
{
    player->media0bject()->play();
}

void VideoWidget::stop()
{
    Phonon::MediaObject *mo = player->media0bject();
    if (mo->state() == Phonon::PausedState) {
        mo->seek(0);
        mo->stop();
    } else {
        mo->stop();
    }
}

void VideoWidget::setControlsVisible(bool on)
{
    slider->setVisible(on);
    volume->setVisible(on);
    tb->setVisible(on);
    chunk_bar->setVisible(player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream && on);
    time_label->setVisible(on);
}

bool VideoWidget::eventFilter(QObject *dst, QEvent *event)
{
    Q_UNUSED(dst);
    if (fullscreen && event->type() == QEvent::MouseMove)
        mouseMoveEvent((QMouseEvent *)event);

    return true;
}

void VideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!fullscreen)
        return;

    bool streaming = player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream;
    if (slider->isVisible()) {
        int bh = height() - slider->height();
        int th = streaming ? chunk_bar->height() : 0;
        if (event->position().y() < bh - 10 && event->position().y() > th + 10) // use a 10 pixel safety buffer to avoid fibrilation
            setControlsVisible(false);
    } else {
        int bh = height() - slider->height();
        int th = streaming ? chunk_bar->height() : 0;
        if (event->position().y() >= bh || event->position().y() <= th)
            setControlsVisible(true);
    }
}

void VideoWidget::setFullScreen(bool on)
{
    if (on) {
        setWindowState(windowState() | Qt::WindowFullScreen);
        setControlsVisible(false);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        setControlsVisible(true);
    }
    fullscreen = on;
    setMouseTracking(fullscreen);
}

void VideoWidget::inhibitScreenSaver(bool on)
{
    org::freedesktop::ScreenSaver screensaver(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"), QDBusConnection::sessionBus());
    org::freedesktop::PowerManagement::Inhibit powerManagement(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                                                               QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"),
                                                               QDBusConnection::sessionBus());
    if (on) {
        QString msg = i18n("KTorrent is playing a video.");
        auto pendingReply = screensaver.Inhibit(QStringLiteral("ktorrent"), msg);
        auto pendingCallWatcher = new QDBusPendingCallWatcher(pendingReply, this);
        connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
            QDBusPendingReply<quint32> reply = *callWatcher;
            if (reply.isValid()) {
                screensaver_cookie = reply.value();
                Out(SYS_MPL | LOG_NOTICE) << "Screensaver inhibited (cookie " << screensaver_cookie << ")" << endl;
            } else
                Out(SYS_GEN | LOG_IMPORTANT) << "Failed to suppress screensaver" << endl;
        });

        auto pendingReply2 = powerManagement.Inhibit(QStringLiteral("ktorrent"), msg);
        auto pendingCallWatcher2 = new QDBusPendingCallWatcher(pendingReply2, this);
        connect(pendingCallWatcher2, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
            QDBusPendingReply<quint32> reply = *callWatcher;
            if (reply.isValid()) {
                screensaver_cookie = reply.value();
                Out(SYS_MPL | LOG_NOTICE) << "PowerManagement inhibited (cookie " << powermanagement_cookie << ")" << endl;
            } else
                Out(SYS_GEN | LOG_IMPORTANT) << "Failed to suppress sleeping" << endl;
        });
    } else {
        auto pendingReply = screensaver.UnInhibit(screensaver_cookie);
        auto pendingCallWatcher = new QDBusPendingCallWatcher(pendingReply, this);
        connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
            QDBusPendingReply<void> reply = *callWatcher;
            if (reply.isValid()) {
                screensaver_cookie = 0;
                Out(SYS_MPL | LOG_NOTICE) << "Screensaver uninhibited" << endl;
            } else
                Out(SYS_MPL | LOG_IMPORTANT) << "Failed uninhibit screensaver" << endl;
        });

        auto pendingReply2 = powerManagement.UnInhibit(powermanagement_cookie);
        auto pendingCallWatcher2 = new QDBusPendingCallWatcher(pendingReply2, this);
        connect(pendingCallWatcher2, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
            QDBusPendingReply<void> reply = *callWatcher;
            if (reply.isValid()) {
                powermanagement_cookie = 0;
                Out(SYS_MPL | LOG_NOTICE) << "Power management uninhibited" << endl;
            } else
                Out(SYS_MPL | LOG_IMPORTANT) << "Failed uninhibit power management" << endl;
        });
    }
}

void VideoWidget::timerTick(qint64 time)
{
    time_label->setText(formatTime(time, player->media0bject()->totalTime()));
    if (chunk_bar->isVisible())
        chunk_bar->timeElapsed(time);
}

QString VideoWidget::formatTime(qint64 cur, qint64 total)
{
    QTime ct(cur / (60 * 60 * 1000), (cur / (60 * 1000)) % 60, (cur / 1000) % 60, cur % 1000);
    QTime tt(total / (60 * 60 * 1000), (total / (60 * 1000)) % 60, (total / 1000) % 60, total % 1000);
    return QStringLiteral(" %1 / %2 ").arg(ct.toString(QStringLiteral("hh:mm:ss")), tt.toString(QStringLiteral("hh:mm:ss")));
}

void VideoWidget::playing(const MediaFileRef &mfile)
{
    bool stream = player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream;
    if (fullscreen && stream)
        chunk_bar->setVisible(slider->isVisible());
    else
        chunk_bar->setVisible(stream);

    chunk_bar->setMediaFile(mfile);
}

void VideoWidget::enableActions(unsigned int flags)
{
    play_action->setEnabled(flags & kt::MEDIA_PLAY);
    stop_action->setEnabled(flags & kt::MEDIA_STOP);
}

}

#include "moc_videowidget.cpp"
