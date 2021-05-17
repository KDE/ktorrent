/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediaplayeractivity.h"

#include <QAction>
#include <QBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSplitter>
#include <QToolButton>

#include <KActionCollection>
#include <KLocalizedString>
#include <KToggleAction>

#include "mediacontroller.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"
#include "mediaview.h"
#include "playlist.h"
#include "playlistwidget.h"
#include "videowidget.h"
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
MediaPlayerActivity::MediaPlayerActivity(CoreInterface *core, KActionCollection *ac, QWidget *parent)
    : Activity(i18n("Media Player"), QStringLiteral("applications-multimedia"), 90, parent)
    , ac(ac)
{
    action_flags = 0;
    video = 0;
    play_action = pause_action = stop_action = prev_action = next_action = 0;
    fullscreen_mode = false;

    media_model = new MediaModel(core, this);
    media_player = new MediaPlayer(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    QWidget *tab = new QWidget(tabs);
    tabs->addTab(tab, QIcon::fromTheme(QStringLiteral("applications-multimedia")), i18n("Media Player"));
    QVBoxLayout *vbox = new QVBoxLayout(tab);

    splitter = new QSplitter(Qt::Horizontal, tab);
    media_view = new MediaView(media_model, splitter);
    play_list = new PlayListWidget(media_model, media_player, tabs);
    setupActions();
    controller = new MediaController(media_player, ac, tab);

    splitter->addWidget(media_view);
    splitter->addWidget(play_list);
    vbox->addWidget(controller);
    vbox->addWidget(splitter);

    close_button = new QToolButton(tabs);
    tabs->setCornerWidget(close_button, Qt::TopRightCorner);
    close_button->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    close_button->setEnabled(false);
    connect(close_button, &QToolButton::clicked, this, &MediaPlayerActivity::closeTab);

    // tabs->setTabBarHidden(true);
    tabs->setTabBarAutoHide(true);

    connect(core, &CoreInterface::torrentAdded, media_model, &MediaModel::onTorrentAdded);
    connect(core, &CoreInterface::torrentRemoved, media_model, &MediaModel::onTorrentRemoved);
    connect(media_player, &MediaPlayer::enableActions, this, &MediaPlayerActivity::enableActions);
    connect(media_player, &MediaPlayer::openVideo, this, &MediaPlayerActivity::openVideo);
    connect(media_player, &MediaPlayer::closeVideo, this, &MediaPlayerActivity::closeVideo);
    connect(media_player, &MediaPlayer::aboutToFinish, this, &MediaPlayerActivity::aboutToFinishPlaying);
    connect(play_list, &PlayListWidget::fileSelected, this, &MediaPlayerActivity::onSelectionChanged);
    connect(media_view, &MediaView::doubleClicked, this, &MediaPlayerActivity::onDoubleClicked);
    connect(play_list, &PlayListWidget::randomModeActivated, this, &MediaPlayerActivity::randomPlayActivated);
    connect(play_list, qOverload<const MediaFileRef &>(&PlayListWidget::doubleClicked), this, qOverload<const MediaFileRef &>(&MediaPlayerActivity::play));
    connect(play_list, &PlayListWidget::enableNext, next_action, &QAction::setEnabled);
    connect(tabs, &QTabWidget::currentChanged, this, &MediaPlayerActivity::currentTabChanged);
}

MediaPlayerActivity::~MediaPlayerActivity()
{
    if (fullscreen_mode)
        setVideoFullScreen(false);
}

void MediaPlayerActivity::setupActions()
{
    play_action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), i18n("Play"), this);
    connect(play_action, &QAction::triggered, this, qOverload<>(&MediaPlayerActivity::play));
    ac->addAction(QStringLiteral("media_play"), play_action);

    pause_action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-pause")), i18n("Pause"), this);
    connect(pause_action, &QAction::triggered, this, &MediaPlayerActivity::pause);
    ac->addAction(QStringLiteral("media_pause"), pause_action);

    stop_action = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-stop")), i18n("Stop"), this);
    connect(stop_action, &QAction::triggered, this, &MediaPlayerActivity::stop);
    ac->addAction(QStringLiteral("media_stop"), stop_action);

    prev_action = new QAction(QIcon::fromTheme(QStringLiteral("media-skip-backward")), i18n("Previous"), this);
    connect(prev_action, &QAction::triggered, this, &MediaPlayerActivity::prev);
    ac->addAction(QStringLiteral("media_prev"), prev_action);

    next_action = new QAction(QIcon::fromTheme(QStringLiteral("media-skip-forward")), i18n("Next"), this);
    connect(next_action, &QAction::triggered, this, &MediaPlayerActivity::next);
    ac->addAction(QStringLiteral("media_next"), next_action);

    show_video_action = new KToggleAction(QIcon::fromTheme(QStringLiteral("video-x-generic")), i18n("Show Video"), this);
    connect(show_video_action, &QAction::toggled, this, &MediaPlayerActivity::showVideo);
    ac->addAction(QStringLiteral("show_video"), show_video_action);

    add_media_action = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("Add Media"), this);
    connect(add_media_action, &QAction::triggered, play_list, &PlayListWidget::addMedia);
    ac->addAction(QStringLiteral("add_media"), add_media_action);

    clear_action = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18n("Clear Playlist"), this);
    connect(clear_action, &QAction::triggered, play_list, &PlayListWidget::clearPlayList);
    ac->addAction(QStringLiteral("clear_play_list"), clear_action);

    QAction *tfs = new QAction(QIcon::fromTheme(QStringLiteral("view-fullscreen")), i18n("Toggle Fullscreen"), this);
    tfs->setCheckable(true);
    ac->addAction(QStringLiteral("video_fullscreen"), tfs);
    ac->setDefaultShortcut(tfs, QKeySequence(Qt::Key_F));
}

void MediaPlayerActivity::openVideo()
{
    QString path = media_player->getCurrentSource().path();
    int idx = path.lastIndexOf(bt::DirSeparator());
    if (idx >= 0)
        path = path.mid(idx + 1);

    if (path.isEmpty())
        path = i18n("Media Player");

    if (video) {
        int idx = tabs->indexOf(video);
        tabs->setTabText(idx, path);
        tabs->setCurrentIndex(idx);
    } else {
        video = new VideoWidget(media_player, ac, 0);
        connect(video, &VideoWidget::toggleFullScreen, this, &MediaPlayerActivity::setVideoFullScreen);
        int idx = tabs->addTab(video, QIcon::fromTheme(QStringLiteral("video-x-generic")), path);
        tabs->setTabToolTip(idx, i18n("Movie player"));
        tabs->setCurrentIndex(idx);
    }
    // tabs->setTabBarHidden(false);

    if (!show_video_action->isChecked())
        show_video_action->setChecked(true);
}

void MediaPlayerActivity::closeVideo()
{
    if (video) {
        tabs->removeTab(tabs->indexOf(video));
        if (show_video_action->isChecked())
            show_video_action->setChecked(false);
        // tabs->setTabBarHidden(true);
        video->deleteLater();
        video = 0;
    }
}

void MediaPlayerActivity::showVideo(bool on)
{
    if (on)
        openVideo();
    else
        closeVideo();
}

void MediaPlayerActivity::play()
{
    if (media_player->paused()) {
        media_player->resume();
    } else {
        curr_item = play_list->play();
        if (curr_item.isValid()) {
            bool random = play_list->randomOrder();
            QModelIndex n = play_list->next(curr_item, random);
            next_action->setEnabled(n.isValid());
        }
    }
}

void MediaPlayerActivity::play(const MediaFileRef &file)
{
    media_player->play(file);
    QModelIndex idx = play_list->indexForFile(file.path());
    if (idx.isValid()) {
        curr_item = idx;
        bool random = play_list->randomOrder();
        QModelIndex n = play_list->next(curr_item, random);
        next_action->setEnabled(n.isValid());
    }
}

void MediaPlayerActivity::onDoubleClicked(const MediaFileRef &file)
{
    if (bt::Exists(file.path())) {
        play(file);
    }
}

void MediaPlayerActivity::pause()
{
    media_player->pause();
}

void MediaPlayerActivity::stop()
{
    media_player->stop();
}

void MediaPlayerActivity::prev()
{
    media_player->prev();
}

void MediaPlayerActivity::next()
{
    bool random = play_list->randomOrder();
    QModelIndex n = play_list->next(curr_item, random);
    if (!n.isValid())
        return;

    QString path = play_list->fileForIndex(n);
    if (bt::Exists(path)) {
        media_player->play(path);
        curr_item = n;
        n = play_list->next(curr_item, random);
        next_action->setEnabled(n.isValid());
    }
}

void MediaPlayerActivity::enableActions(unsigned int flags)
{
    pause_action->setEnabled(flags & kt::MEDIA_PAUSE);
    stop_action->setEnabled(flags & kt::MEDIA_STOP);
    play_action->setEnabled(false);

    QModelIndex idx = play_list->selectedItem();
    if (idx.isValid()) {
        PlayList *pl = play_list->playList();
        MediaFileRef file = pl->fileForIndex(idx);
        if (bt::Exists(file.path()))
            play_action->setEnabled((flags & kt::MEDIA_PLAY) || file != media_player->getCurrentSource());
        else
            play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
    } else
        play_action->setEnabled(flags & kt::MEDIA_PLAY);

    prev_action->setEnabled(flags & kt::MEDIA_PREV);
    action_flags = flags;
}

void MediaPlayerActivity::onSelectionChanged(const MediaFileRef &file)
{
    if (bt::Exists(file.path()))
        play_action->setEnabled((action_flags & kt::MEDIA_PLAY) || file != media_player->getCurrentSource());
    else if (!file.path().isEmpty())
        play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
    else
        play_action->setEnabled(false);
}

void MediaPlayerActivity::randomPlayActivated(bool on)
{
    QModelIndex next = play_list->next(curr_item, on);
    next_action->setEnabled(next.isValid());
}

void MediaPlayerActivity::aboutToFinishPlaying()
{
    bool random = play_list->randomOrder();
    QModelIndex n = play_list->next(curr_item, random);
    if (!n.isValid())
        return;

    QString path = play_list->fileForIndex(n);
    if (bt::Exists(path)) {
        media_player->queue(path);
        curr_item = n;
        n = play_list->next(curr_item, random);
        next_action->setEnabled(n.isValid());
    }
}

void MediaPlayerActivity::closeTab()
{
    if (video != tabs->currentWidget())
        return;

    stop();
    closeVideo();
}

void MediaPlayerActivity::setVideoFullScreen(bool on)
{
    if (!video)
        return;

    if (on && !fullscreen_mode) {
        tabs->removeTab(tabs->indexOf(video));
        video->setParent(0);
        video->setFullScreen(true);
        video->show();
        fullscreen_mode = true;
    } else if (!on && fullscreen_mode) {
        video->hide();
        video->setFullScreen(false);

        QString path = media_player->getCurrentSource().path();
        int idx = path.lastIndexOf(bt::DirSeparator());
        if (idx >= 0)
            path = path.mid(idx + 1);

        if (path.isEmpty())
            path = i18n("Media Player");

        idx = tabs->addTab(video, QIcon::fromTheme(QStringLiteral("video-x-generic")), path);
        tabs->setTabToolTip(idx, i18n("Movie player"));
        tabs->setCurrentIndex(idx);
        fullscreen_mode = false;
    }
}

void MediaPlayerActivity::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MediaPlayerActivity");
    g.writeEntry("splitter_state", splitter->saveState());
    play_list->saveState(cfg);
    play_list->playList()->save(kt::DataDir() + QLatin1String("playlist"));

    media_view->saveState(cfg);
}

void MediaPlayerActivity::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MediaPlayerActivity");
    QByteArray d = g.readEntry("splitter_state", QByteArray());
    if (!d.isEmpty())
        splitter->restoreState(d);

    play_list->loadState(cfg);
    if (bt::Exists(kt::DataDir() + QLatin1String("playlist")))
        play_list->playList()->load(kt::DataDir() + QLatin1String("playlist"));

    QModelIndex next = play_list->next(curr_item, play_list->randomOrder());
    next_action->setEnabled(next.isValid());

    media_view->loadState(cfg);
}

void MediaPlayerActivity::currentTabChanged(int idx)
{
    close_button->setEnabled(idx != 0);
}

}
