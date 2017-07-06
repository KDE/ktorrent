/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef FILESELECTDLG_H
#define FILESELECTDLG_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include <KSharedConfig>

#include <util/constants.h>
#include "ui_fileselectdlg.h"


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

    public:
        FileSelectDlg(kt::QueueManager* qman, kt::GroupManager* gman, const QString& group_hint, QWidget* parent);
        ~FileSelectDlg();

        int execute(bt::TorrentInterface* tc, bool* start, bool* skip_check, const QString& location_hint);

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

    protected slots:
        void reject() override;
        void accept() override;
        void selectAll();
        void selectNone();
        void invertSelection();
        void updateSizeLabels();
        void onCodecChanged(const QString& text);
        void groupActivated(int idx);
        void fileTree(bool on);
        void fileList(bool on);
        void setShowFileTree(bool on);
        void setFilter(const QString& filter);
        void updateExistingFiles();
        void moveCompletedToggled(bool on);
        QMenu* createHistoryMenu(const QStringList& urls, const char* slot);
        void clearDownloadLocationHistory();
        void clearMoveOnCompletionLocationHistory();
        void downloadLocationHistoryTriggered(QAction* act);
        void moveOnCompletionLocationHistoryTriggered(QAction* act);
        void downloadLocationChanged(const QString& path);

    private:
        void populateFields(const QString& location_hint);
        void loadGroups();

    private:
        bt::TorrentInterface* tc;
        TorrentFileModel* model;
        kt::QueueManager* qman;
        kt::GroupManager* gman;
        bool* start;
        bool* skip_check;
        QList<int> encodings;
        kt::Group* initial_group;
        bool show_file_tree;
        QSortFilterProxyModel* filter_model;
        QStringList download_location_history;
        QStringList move_on_completion_location_history;
        bt::Uint64 already_downloaded;
    };
}

#endif

