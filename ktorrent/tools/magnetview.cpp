/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "magnetview.h"
#include "magnetmodel.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QBoxLayout>
#include <QClipboard>
#include <QGuiApplication>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QTreeView>

#include <torrent/magnetmanager.h>

namespace kt
{
MagnetView::MagnetView(MagnetManager *magnetManager, QWidget *parent)
    : QWidget(parent)
    , mman(magnetManager)
{
    model = new MagnetModel(magnetManager, this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    // magnets view
    view = new QTreeView(this);
    view->setModel(model);
    view->setUniformRowHeights(true);
    view->setRootIsDecorated(false);
    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSortingEnabled(false);
    view->setAllColumnsShowFocus(true);
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QTreeView::customContextMenuRequested, this, &MagnetView::showContextMenu);
    layout->addWidget(view);

    // context menu
    menu = new QMenu(this);
    start = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-start")), i18n("Start Magnet"), this, &MagnetView::startMagnetDownload);
    stop = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-stop")), i18n("Stop Magnet"), this, &MagnetView::stopMagnetDownload);
    copy_url = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy Magnet URL"), this, &MagnetView::copyMagnetUrl);
    menu->addSeparator();
    remove = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-remove")), i18n("Remove Magnet"), this, &MagnetView::removeMagnetDownload);
}

MagnetView::~MagnetView()
{
}

void MagnetView::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MagnetView");
    QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
    if (!s.isEmpty()) {
        QHeaderView *v = view->header();
        v->restoreState(s);
    }
}

void MagnetView::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MagnetView");
    g.writeEntry("state", view->header()->saveState().toBase64());
}

void MagnetView::showContextMenu(QPoint p)
{
    const QModelIndexList idx_list = view->selectionModel()->selectedRows();

    start->setEnabled(false);
    stop->setEnabled(false);
    remove->setEnabled(idx_list.count() > 0);

    for (const QModelIndex &idx : idx_list) {
        if (!mman->isStopped(idx.row()))
            stop->setEnabled(true);
        else
            start->setEnabled(true);
    }
    menu->popup(view->viewport()->mapToGlobal(p));
}

void MagnetView::removeMagnetDownload()
{
    QModelIndexList idx_list = view->selectionModel()->selectedRows();
    if (!idx_list.isEmpty())
        mman->removeMagnets(idx_list.front().row(), idx_list.size());
}

void MagnetView::startMagnetDownload()
{
    QModelIndexList idx_list = view->selectionModel()->selectedRows();
    if (!idx_list.isEmpty()) {
        mman->start(idx_list.front().row(), idx_list.size());
        view->clearSelection();
    }
}

void MagnetView::stopMagnetDownload()
{
    QModelIndexList idx_list = view->selectionModel()->selectedRows();
    if (!idx_list.isEmpty()) {
        mman->stop(idx_list.front().row(), idx_list.size());
        view->clearSelection();
    }
}

void MagnetView::copyMagnetUrl()
{
    QStringList sl;
    const QModelIndexList idx_list = view->selectionModel()->selectedRows();
    for (const QModelIndex &idx : idx_list) {
        if (const MagnetDownloader *md = mman->getMagnetDownloader(idx.row())) {
            sl.append(md->magnetLink().toString());
        }
    }
    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(sl.join(QStringLiteral("\n")));
    }
}

void MagnetView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        removeMagnetDownload();
        event->accept();
    } else
        QWidget::keyPressEvent(event);
}
}
