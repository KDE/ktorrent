/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFILEVIEW_H
#define KTFILEVIEW_H

#include <KSharedConfig>
#include <QTreeView>

#include <interfaces/torrentinterface.h>
#include <util/constants.h>

class QLineEdit;
class QMenu;
class QSortFilterProxyModel;
class QToolBar;

namespace bt
{
class TorrentFileInterface;
}

namespace kt
{
class TorrentFileModel;

/**
    @author Joris Guisson <joris.guisson@gmail.com>
*/
class FileView : public QWidget
{
    Q_OBJECT
public:
    FileView(QWidget *parent);
    ~FileView() override;

    void changeTC(bt::TorrentInterface *tc);
    void setShowListOfFiles(bool on);
    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);
    void update();
    void filePercentageChanged(bt::TorrentFileInterface *file, float percentage);
    void filePreviewChanged(bt::TorrentFileInterface *file, bool preview);

public Q_SLOTS:
    void onTorrentRemoved(bt::TorrentInterface *tc);

private Q_SLOTS:
    void showContextMenu(const QPoint &p);
    void onDoubleClicked(const QModelIndex &index);
    void onMissingFileMarkedDND(bt::TorrentInterface *tc);

private:
    void changePriority(bt::Priority newpriority);
    void expandCollapseTree(const QModelIndex &idx, bool expand);
    void expandCollapseSelected(bool expand);
    void setupActions();

private Q_SLOTS:
    void open();
    void openWith();
    void downloadFirst();
    void downloadLast();
    void downloadNormal();
    void doNotDownload();
    void deleteFiles();
    void moveFiles();
    void collapseTree();
    void expandTree();
    void showTree();
    void showList();
    void setFilter(const QString &f);
    void checkFile();

private:
    bt::TorrentInterface::WPtr curr_tc;
    TorrentFileModel *model;

    QMenu *context_menu;
    QAction *open_action;
    QAction *open_with_action;
    QAction *download_first_action;
    QAction *download_normal_action;
    QAction *download_last_action;
    QAction *dnd_action;
    QAction *delete_action;
    QAction *move_files_action;
    QAction *collapse_action;
    QAction *expand_action;
    QAction *show_tree_action;
    QAction *show_list_action;
    QAction *show_filter_action;
    QAction *check_data;

    QString preview_path;
    bool show_list_of_files;
    QMap<bt::TorrentInterface *, QByteArray> expanded_state_map;
    QSortFilterProxyModel *proxy_model;
    bool header_state_loaded;

    QTreeView *view;
    QToolBar *toolbar;
    QLineEdit *filter;
};

}

#endif
