/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILESELECTDLG_H
#define FILESELECTDLG_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include <KSharedConfig>

#include "ui_fileselectdlg.h"
#include <util/constants.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class GroupManager;
class QueueManager;
class TorrentFileModel;
class Group;

/**
 * @author Joris Guisson
 *
 * Dialog to select which files to download from a multifile torrent.
 */
class FileSelectDlg : public QDialog, public Ui_FileSelectDlg
{
    Q_OBJECT

    typedef void (FileSelectDlg::*Func)(QAction *);

public:
    FileSelectDlg(kt::QueueManager *qman, kt::GroupManager *gman, const QString &group_hint, QWidget *parent);
    ~FileSelectDlg() override;

    int execute(bt::TorrentInterface *tc, bool *start, bool *skip_check, const QString &location_hint);

    /// Which group did the user select
    QString selectedGroup() const;

    /**
     * Load the state of the dialog
     */
    void loadState(KSharedConfigPtr cfg);

    /**
     * Save the state of the dialog
     */
    void saveState(KSharedConfigPtr cfg);

protected Q_SLOTS:
    void reject() override;
    void accept() override;
    void selectAll();
    void selectNone();
    void invertSelection();
    void updateSizeLabels();
    void onCodecChanged(const int index);
    void groupActivated(int idx);
    void fileTree(bool on);
    void fileList(bool on);
    void setShowFileTree(bool on);
    void setFilter(const QString &filter);
    void updateExistingFiles();
    void moveCompletedToggled(bool on);
    QMenu *createHistoryMenu(const QStringList &urls, Func slot);
    void clearDownloadLocationHistory();
    void clearMoveOnCompletionLocationHistory();
    void downloadLocationHistoryTriggered(QAction *act);
    void moveOnCompletionLocationHistoryTriggered(QAction *act);
    void downloadLocationChanged(const QString &path);

private:
    void populateFields(const QString &location_hint);
    void loadGroups();

private:
    bt::TorrentInterface *tc;
    TorrentFileModel *model;
    kt::QueueManager *qman;
    kt::GroupManager *gman;
    bool *start;
    bool *skip_check;
    QList<int> encodings;
    kt::Group *initial_group;
    bool show_file_tree;
    QSortFilterProxyModel *filter_model;
    QStringList download_location_history;
    QStringList move_on_completion_location_history;
    bt::Uint64 already_downloaded;
};
}

#endif
