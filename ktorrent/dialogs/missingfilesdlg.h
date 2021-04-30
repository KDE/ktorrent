/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTMISSINGFILESDLG_H
#define KTMISSINGFILESDLG_H

#include "ui_missingfilesdlg.h"
#include <QDialog>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
    Dialog to show when files are missing.
*/
class MissingFilesDlg : public QDialog, public Ui_MissingFilesDlg
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param text Text to show above file list
     * @param missing The list of missing files
     * @param tc The torrent
     * @param parent The parent widget
     */
    MissingFilesDlg(const QString &text, const QStringList &missing, bt::TorrentInterface *tc, QWidget *parent);
    ~MissingFilesDlg() override;

    enum ReturnCode {
        RECREATE,
        DO_NOT_DOWNLOAD,
        CANCEL,
        NEW_LOCATION_SELECTED,
    };

    /**
     * Execute the dialog
     * @return What to do
     */
    ReturnCode execute();

private Q_SLOTS:
    void dndPressed();
    void recreatePressed();
    void cancelPressed();
    void selectNewPressed();

private:
    ReturnCode ret;
    bt::TorrentInterface *tc;
};

}

#endif
