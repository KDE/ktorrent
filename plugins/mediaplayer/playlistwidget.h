/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QTreeView>

#include <KSharedConfig>

#include "mediafile.h"

class QSortFilterProxyModel;

namespace kt
{
class PlayList;
class MediaPlayer;
class MediaFileCollection;

class PlayListWidget : public QWidget
{
    Q_OBJECT
public:
    PlayListWidget(MediaFileCollection *collection, MediaPlayer *player, QWidget *parent);
    ~PlayListWidget() override;

    /// Get the play list
    PlayList *playList()
    {
        return play_list;
    }

    /// Get the current selected item
    QModelIndex selectedItem() const;

    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

    /// Get the next item to play, if idx is invalid return the first playable item
    QModelIndex next(const QModelIndex &idx, bool random) const;

    /// Get the file of a given index
    QString fileForIndex(const QModelIndex &index) const;

    /// Get the index of a file
    QModelIndex indexForFile(const QString &file) const;

    /// Is random mode activated ?
    bool randomOrder() const
    {
        return random_mode->isChecked();
    }

public Q_SLOTS:
    QModelIndex play();
    void addMedia();
    void clearPlayList();

private Q_SLOTS:
    void onSelectionChanged(const QItemSelection &s, const QItemSelection &d);
    void doubleClicked(const QModelIndex &index);
    void showContextMenu(QPoint pos);
    void removeFiles();
    void onItemsDropped();

Q_SIGNALS:
    void fileSelected(const MediaFileRef &file);
    void doubleClicked(const MediaFileRef &file);
    void randomModeActivated(bool random);
    void enableNext(bool on);

private:
    QModelIndex next(const QModelIndex &idx) const;
    QModelIndex randomNext(const QModelIndex &idx) const;

private:
    MediaPlayer *player;
    PlayList *play_list;
    QToolBar *tool_bar;
    QTreeView *view;
    QCheckBox *random_mode;

    QMenu *menu;
    QSortFilterProxyModel *proxy_model;
    MediaFileCollection *collection;
};
}

#endif // PLAYLISTWIDGET_H
