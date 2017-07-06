/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef KTQUEUEMANAGERWIDGET_H
#define KTQUEUEMANAGERWIDGET_H

#include <QWidget>
#include <KSharedConfig>

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
        QueueManagerWidget(QueueManager* qman, QWidget* parent);
        ~QueueManagerWidget();

        /// Save the widget state
        void saveState(KSharedConfigPtr cfg);
        /// Load the widget state
        void loadState(KSharedConfigPtr cfg);
        /// Update the widget
        void update();

    public slots:
        void onTorrentAdded(bt::TorrentInterface* tc);
        void onTorrentRemoved(bt::TorrentInterface* tc);

    private slots:
        void moveUpClicked();
        void moveDownClicked();
        void moveTopClicked();
        void moveBottomClicked();
        void searchTextChanged(const QString& t);
        void showSearch(bool on);
        void showDownloads(bool on);
        void showUploads(bool on);
        void showNotQueued(bool on);
        void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    private:
        bool indexVisible(const QModelIndex& idx);
        void updateButtons();

    private:
        QueueManagerModel* model;
        QueueManager* qman;
        QTreeView* view;
        QToolBar* toolbar;
        QLineEdit* search;

        QAction* show_search;
        QAction* move_top;
        QAction* move_up;
        QAction* move_down;
        QAction* move_bottom;

        QAction* show_uploads;
        QAction* show_downloads;
        QAction* show_not_queued;
    };
}

#endif
