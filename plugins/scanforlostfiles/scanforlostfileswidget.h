/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCANFORLOSTFILESWIDGET_H
#define KTSCANFORLOSTFILESWIDGET_H

#include "scanforlostfilesplugin.h"
#include "ui_scanforlostfileswidget.h"
#include <interfaces/activity.h>

class QFileSystemModel;
class QMenu;

namespace kt
{
class FSProxyModel;
class ScanForLostFilesThread;

/**
 * ScanForLostFiles plugin widget
 */
class ScanForLostFilesWidget : public Activity, public Ui::ScanForLostFilesWidget
{
    Q_OBJECT

public:
    ScanForLostFilesWidget(ScanForLostFilesPlugin *plugin, QWidget *parent = nullptr);
    ~ScanForLostFilesWidget() override;

private Q_SLOTS:
    void on_btnScanFolder_clicked();
    void on_btnExpandAll_clicked();
    void on_btnCollapseAll_clicked();
    void on_actionDelete_on_disk_triggered();
    void on_treeView_customContextMenuRequested(const QPoint &pos);

    void directoryLoaded(const QString &path);

private:
    void setupModels();

private:
    ScanForLostFilesPlugin *m_plugin;
    QFileSystemModel *m_model;
    FSProxyModel *m_proxy;
    QMenu *m_menu;
    ScanForLostFilesThread *m_thread;
};

}

#endif
