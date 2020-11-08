/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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

#ifndef KTSCANFORLOSTFILESWIDGET_H
#define KTSCANFORLOSTFILESWIDGET_H

#include <interfaces/activity.h>
#include "scanforlostfilesplugin.h"
#include "ui_scanforlostfileswidget.h"

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
    ScanForLostFilesWidget(ScanForLostFilesPlugin* plugin, QWidget* parent = nullptr);
    ~ScanForLostFilesWidget() override;

private Q_SLOTS:
    void on_btnScanFolder_clicked();
    void on_btnExpandAll_clicked();
    void on_btnCollapseAll_clicked();
    void on_actionDelete_on_disk_triggered();
    void on_treeView_customContextMenuRequested(const QPoint &pos);

    void directoryLoaded(const QString& path);

private:
    void setupModels();

private:
    ScanForLostFilesPlugin* m_plugin;
    QFileSystemModel* m_model;
    FSProxyModel* m_proxy;
    QMenu* m_menu;
    ScanForLostFilesThread* m_thread;
};

}

#endif
