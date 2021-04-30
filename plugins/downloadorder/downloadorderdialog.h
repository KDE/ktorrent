/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDOWNLOADORDERDIALOG_H
#define KTDOWNLOADORDERDIALOG_H

#include "ui_downloadorderwidget.h"
#include <QDialog>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class DownloadOrderPlugin;
class DownloadOrderModel;

/**
    Dialog to manipulate the download order.
*/
class DownloadOrderDialog : public QDialog, public Ui_DownloadOrderWidget
{
    Q_OBJECT
public:
    DownloadOrderDialog(DownloadOrderPlugin *plugin, bt::TorrentInterface *tor, QWidget *parent);
    ~DownloadOrderDialog() override;

private Q_SLOTS:
    void commitDownloadOrder();
    void moveUp();
    void moveDown();
    void moveTop();
    void moveBottom();
    void itemSelectionChanged(const QItemSelection &new_sel, const QItemSelection &old_sel);
    void customOrderEnableToggled(bool on);
    void search(const QString &text);

private:
    bt::TorrentInterface *tor;
    DownloadOrderPlugin *plugin;
    DownloadOrderModel *model;
};

}

#endif
