/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TORRENTACTIVITY_H
#define TORRENTACTIVITY_H

#include <QSplitter>
#include <interfaces/torrentactivityinterface.h>

class QAction;
class KToggleAction;

namespace kt
{
class MagnetView;
class GUI;
class Core;
class View;
class GroupView;
class QueueManagerWidget;
class TabBarWidget;
class Group;
class TorrentSearchBar;
class GroupSwitcher;

/**
 * Activity which manages torrents.
 */
class TorrentActivity : public TorrentActivityInterface
{
    Q_OBJECT
public:
    TorrentActivity(Core *core, GUI *gui, QWidget *parent);
    ~TorrentActivity() override;

    /// Get the group view
    GroupView *getGroupView()
    {
        return group_view;
    }

    void loadState(KSharedConfigPtr cfg);
    void saveState(KSharedConfigPtr cfg);
    const bt::TorrentInterface *getCurrentTorrent() const override;
    bt::TorrentInterface *getCurrentTorrent() override;
    void updateActions() override;
    void addToolWidget(QWidget *widget, const QString &text, const QString &icon, const QString &tooltip) override;
    void removeToolWidget(QWidget *widget) override;
    Group *addNewGroup() override;

    /// Update the activity
    void update();

    /// Setup all actions
    void setupActions();

public Q_SLOTS:
    /**
     * Called by the ViewManager when the current torrent has changed
     * @param tc The torrent
     * */
    void currentTorrentChanged(bt::TorrentInterface *tc);

    /**
        Hide or show the group view
    */
    void setGroupViewVisible(bool visible);

    /**
     * The suspended state has changed
     * @param suspended
     */
    void onSuspendedStateChanged(bool suspended);

private Q_SLOTS:
    void startAllTorrents();
    void stopAllTorrents();
    void suspendQueue(bool suspend);
    void queueOrdered();

private:
    Core *core;
    GUI *gui;
    View *view;
    GroupView *group_view;
    GroupSwitcher *group_switcher;
    QueueManagerWidget *qm;
    QSplitter *hsplit;
    QSplitter *vsplit;
    TabBarWidget *tool_views;
    MagnetView *magnet_view;
    TorrentSearchBar *search_bar;

    QAction *start_all_action;
    QAction *stop_all_action;
    KToggleAction *queue_suspend_action;
    QAction *show_group_view_action;
    QAction *filter_torrent_action;
};
}

#endif // TORRENTACTIVITY_H
