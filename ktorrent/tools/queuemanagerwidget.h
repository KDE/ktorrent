/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTQUEUEMANAGERWIDGET_H
#define KTQUEUEMANAGERWIDGET_H

#include <KSharedConfig>
#include <QWidget>

class QItemSelection;
class QModelIndex;
class QToolBar;
class QTreeView;
class QLineEdit;

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class QueueManager;
class QueueManagerModel;

/**
 * @author Joris Guisson
 *
 * Widget for the QueueManager
 */
class QueueManagerWidget : public QWidget
{
    Q_OBJECT
public:
    QueueManagerWidget(QueueManager *qman, QWidget *parent);
    ~QueueManagerWidget() override;

    /// Save the widget state
    void saveState(KSharedConfigPtr cfg);
    /// Load the widget state
    void loadState(KSharedConfigPtr cfg);
    /// Update the widget
    void update();

public Q_SLOTS:
    void onTorrentAdded(bt::TorrentInterface *tc);
    void onTorrentRemoved(bt::TorrentInterface *tc);

private Q_SLOTS:
    void moveUpClicked();
    void moveDownClicked();
    void moveTopClicked();
    void moveBottomClicked();
    void searchTextChanged(const QString &t);
    void showSearch(bool on);
    void showDownloads(bool on);
    void showUploads(bool on);
    void showNotQueued(bool on);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    bool indexVisible(const QModelIndex &idx);
    void updateButtons();

private:
    QueueManagerModel *model;
    QueueManager *qman;
    QTreeView *view;
    QToolBar *toolbar;
    QLineEdit *search;

    QAction *show_search;
    QAction *move_top;
    QAction *move_up;
    QAction *move_down;
    QAction *move_bottom;

    QAction *show_uploads;
    QAction *show_downloads;
    QAction *show_not_queued;
};
}

#endif
