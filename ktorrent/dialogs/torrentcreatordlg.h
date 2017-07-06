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

#ifndef KT_TORRENTCREATORDLG_HH
#define KT_TORRENTCREATORDLG_HH

#include <QDialog>
#include "ui_torrentcreatordlg.h"
#include <torrent/torrentcreator.h>

namespace kt
{
    class StringCompletionModel;
    class Core;
    class GUI;

    /**
     * Dialog to create torrents with
     */
    class TorrentCreatorDlg : public QDialog, public Ui_TorrentCreatorDlg
    {
        Q_OBJECT
    public:
        TorrentCreatorDlg(Core* core, GUI* gui, QWidget* parent);
        ~TorrentCreatorDlg();

    private slots:
        void addTrackerPressed();
        void removeTrackerPressed();
        void moveUpPressed();
        void moveDownPressed();

        void addWebSeedPressed();
        void removeWebSeedPressed();

        void addNodePressed();
        void removeNodePressed();

        void dhtToggled(bool on);

        void nodeTextChanged(const QString& str);
        void nodeSelectionChanged();

        void trackerTextChanged(const QString& str);
        void trackerSelectionChanged();

        void webSeedTextChanged(const QString& str);
        void webSeedSelectionChanged();

        void hashCalculationDone();
        void updateProgressBar();

        void accept() override;
        void reject() override;

    private:
        void loadGroups();
        void loadCompleterData();
        void setProgressBarEnabled(bool on);

    private:
        Core* core;
        GUI* gui;
        StringCompletionModel* tracker_completion;
        StringCompletionModel* webseeds_completion;
        StringCompletionModel* nodes_completion;
        bt::TorrentCreator* mktor;
        QTimer update_timer;
    };
}


#endif
