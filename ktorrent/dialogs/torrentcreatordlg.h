/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_TORRENTCREATORDLG_HH
#define KT_TORRENTCREATORDLG_HH

#include <QDialog>
#include <QTimer>

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
    TorrentCreatorDlg(Core *core, GUI *gui, QWidget *parent);
    ~TorrentCreatorDlg() override;

private Q_SLOTS:
    void addTrackerPressed();
    void removeTrackerPressed();
    void moveUpPressed();
    void moveDownPressed();

    void addWebSeedPressed();
    void removeWebSeedPressed();

    void addNodePressed();
    void removeNodePressed();

    void dhtToggled(bool on);

    void nodeTextChanged(const QString &str);
    void nodeSelectionChanged();

    void trackerTextChanged(const QString &str);
    void trackerSelectionChanged();

    void webSeedTextChanged(const QString &str);
    void webSeedSelectionChanged();

    void hashCalculationDone();
    void updateProgressBar();

    void accept() override;
    void reject() override;

    void selectFile(); // required for radio button for new torrent creation
    void selectDirectory();

private:
    void loadGroups();
    void loadCompleterData();
    void setProgressBarEnabled(bool on);

private:
    Core *core;
    GUI *gui;
    StringCompletionModel *tracker_completion;
    StringCompletionModel *webseeds_completion;
    StringCompletionModel *nodes_completion;
    bt::TorrentCreator *mktor;
    QTimer update_timer;
};
}

#endif
