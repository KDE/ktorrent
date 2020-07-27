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

#include "scanforlostfilesplugin.h"
#include "scanforlostfileswidget.h"
#include <QMenu>
#include <QClipboard>
#include <KRun>
#include <KMessageBox>
#include <KIO/DeleteJob>

#include "scanforlostfilesthread.h"
#include "fsproxymodel.h"
#include <QtGlobal>
#include <QFileSystemModel>
#include <KLocalizedString>

#include <util/functions.h>
#include <groups/groupmanager.h>
#include <interfaces/coreinterface.h>
#include "scanforlostfilespluginsettings.h"


namespace kt
{

    ScanForLostFilesWidget::ScanForLostFilesWidget(ScanForLostFilesPlugin* plugin, QWidget *parent) : Activity(i18n("Scan for lost files"), QStringLiteral("edit-find"), 1000, parent), m_plugin(plugin), m_thread(nullptr)
    {
        setupUi(this);

        m_model = new QFileSystemModel(this);
        m_model->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);

        m_proxy = new FSProxyModel(this);

        connect(cbShowAllFiles, &QCheckBox::stateChanged, [=](int val){
            m_proxy->setFiltered(!val);
            setupModels();
        });

        connect(actionCopy_to_clipboard, &QAction::triggered, [=](){
            QModelIndex m = treeView->currentIndex();
            m = m_proxy->mapToSource(m);
            const QString fname = m_model->fileName(m);
            QGuiApplication::clipboard()->setText(fname);
        });

        connect(actionOpen_file, &QAction::triggered, [=](){
            QModelIndex index = treeView->currentIndex();
            new KRun(QUrl::fromLocalFile(m_model->filePath(m_proxy->mapToSource(index))), 0, true);
        });

        treeView->setSortingEnabled(true);
        m_menu = new QMenu(treeView);
        m_menu->addAction(actionCopy_to_clipboard);
        m_menu->addAction(actionOpen_file);
        m_menu->addAction(actionDelete_on_disk);
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);

        setupModels();

        progressBar->setVisible(false);

        reqFolder->setMode(KFile::Directory | KFile::ExistingOnly);
        connect(reqFolder, &KUrlRequester::urlSelected, btnScanFolder, &QPushButton::click);
        connect(reqFolder, QOverload<>::of(&KUrlRequester::returnPressed), btnScanFolder, &QPushButton::click);
        if (CoreInterface* c = m_plugin->getCore()) {
            if (GroupManager* gm = c->getGroupManager()) {
                if (Group* all = gm->allGroup()) {
                    const QString default_save_location = all->groupPolicy().default_save_location;
                    if (!default_save_location.isEmpty()) {
                        reqFolder->setUrl(QUrl::fromLocalFile(default_save_location));
                    }
                }
            }
        }
    }


    void ScanForLostFilesWidget::setupModels()
    {
        const QString root_folder = reqFolder->text();
        m_proxy->setSourceModel(nullptr);
        treeView->setModel(nullptr);
        m_model->setRootPath(root_folder);
        m_proxy->setSourceModel(m_model);
        treeView->setModel(m_proxy);
        treeView->header()->hideSection(2); // hide Type column
        treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        QModelIndex m = m_model->index(root_folder);
        m = m_proxy->mapFromSource(m);
        if (m.isValid()) {
            treeView->setRootIndex(m);
        }
    }


    ScanForLostFilesWidget::~ScanForLostFilesWidget()
    {}

    void ScanForLostFilesWidget::directoryLoaded(const QString& path)
    {
        QModelIndex m = m_model->index(path);
        if (m_model->canFetchMore(m)) {
            m_model->fetchMore(m);
        }
        treeView->expandAll();
    }

    void ScanForLostFilesWidget::on_btnExpandAll_clicked()
    {
        connect(m_model, &QFileSystemModel::directoryLoaded, this, &ScanForLostFilesWidget::directoryLoaded);
        treeView->expandAll();
    }

    void ScanForLostFilesWidget::on_btnCollapseAll_clicked()
    {
        disconnect(m_model, &QFileSystemModel::directoryLoaded, this, &ScanForLostFilesWidget::directoryLoaded);
        treeView->collapseAll();
    }

    void ScanForLostFilesWidget::on_btnScanFolder_clicked()
    {
        if (treeView->model()) {
            treeView->setModel(nullptr);
        }

        if (m_thread) {
            m_thread->requestInterruption();
            m_thread->terminate();
            m_thread->wait();
            m_thread = nullptr;
            return;
        }

        const QString root_folder = reqFolder->text();

        m_thread = new ScanForLostFilesThread(root_folder, m_plugin->getCore(), this);
        btnScanFolder->setText(i18n("Cancel"));
        progressBar->setVisible(true);


        connect(m_thread, &ScanForLostFilesThread::finished, this, [=]() {
            btnScanFolder->setText(i18n("Scan"));
            progressBar->setVisible(false);
            m_thread->deleteLater();
            m_thread = nullptr;
        },  Qt::QueuedConnection);

        connect(m_thread, &ScanForLostFilesThread::filterReady, this, [=](QSet<QString>* filter){
            if (filter) {
                m_proxy->setFilter(filter);
                setupModels();
            }
        },  Qt::QueuedConnection);


        m_thread->start();
    }

    void ScanForLostFilesWidget::on_actionDelete_on_disk_triggered()
    {
        QModelIndexList sel = treeView->selectionModel()->selectedRows();
        int n = sel.count();
        if (n == 1) // single item can be a directory
        {
            if (m_model->fileInfo(m_proxy->mapToSource(sel.front())).isDir())
                n++;
        }

        QString msg = i18np("You will lose all data in this file, are you sure you want to do this?",
                            "You will lose all data in these files, are you sure you want to do this?", n);

        QList<QUrl> to_del;
        if (KMessageBox::warningYesNo(0, msg) == KMessageBox::Yes) {
            for (const QModelIndex& m : sel) {
                to_del.append(QUrl::fromLocalFile(m_model->filePath(m_proxy->mapToSource(m))));
            }
            KIO::del(to_del);
        }

    }

    void ScanForLostFilesWidget::on_treeView_customContextMenuRequested(const QPoint &pos)
    {
        actionOpen_file->setEnabled(treeView->currentIndex().isValid());
        actionDelete_on_disk->setEnabled(treeView->currentIndex().isValid() || treeView->selectionModel()->selectedRows().count());

        m_menu->exec(treeView->mapToGlobal(pos));
    }


}

