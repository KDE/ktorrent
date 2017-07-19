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

#include "queuemanagerwidget.h"

#include <QAction>
#include <QBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <QToolBar>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardGuiItem>

#include <torrent/queuemanager.h>
#include <util/log.h>
#include "queuemanagermodel.h"


using namespace bt;

namespace kt
{

    QueueManagerWidget::QueueManagerWidget(QueueManager* qman, QWidget* parent) : QWidget(parent), qman(qman)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);
        QVBoxLayout* vbox = new QVBoxLayout();
        vbox->setMargin(0);
        vbox->setSpacing(0);
        view = new QTreeView(this);
        view->setUniformRowHeights(true);
        toolbar = new QToolBar(this);
        toolbar->setOrientation(Qt::Vertical);
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        layout->addWidget(toolbar);

        search = new QLineEdit(this);
        search->setPlaceholderText(i18n("Search"));
        search->setClearButtonEnabled(true);
        search->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(search, &QLineEdit::textChanged, this, &QueueManagerWidget::searchTextChanged);
        search->hide();
        vbox->addWidget(search);
        vbox->addWidget(view);
        layout->addLayout(vbox);

        show_search = toolbar->addAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Show Search"));
        show_search->setToolTip(i18n("Show or hide the search bar"));
        show_search->setCheckable(true);
        connect(show_search, &QAction::toggled, this, &QueueManagerWidget::showSearch);

        move_top = toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-top")), i18n("Move Top"), this, SLOT(moveTopClicked()));
        move_top->setToolTip(i18n("Move a torrent to the top of the queue"));

        move_up = toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-up")), i18n("Move Up"), this, SLOT(moveUpClicked()));
        move_up->setToolTip(i18n("Move a torrent up in the queue"));

        move_down = toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-down")), i18n("Move Down"), this, SLOT(moveDownClicked()));
        move_down->setToolTip(i18n("Move a torrent down in the queue"));

        move_bottom = toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-bottom")), i18n("Move Bottom"), this, SLOT(moveBottomClicked()));
        move_bottom->setToolTip(i18n("Move a torrent to the bottom of the queue"));

        show_downloads = toolbar->addAction(QIcon::fromTheme(QStringLiteral("arrow-down")), i18n("Show Downloads"));
        show_downloads->setToolTip(i18n("Show all downloads"));
        show_downloads->setCheckable(true);
        connect(show_downloads, &QAction::toggled, this, &QueueManagerWidget::showDownloads);

        show_uploads = toolbar->addAction(QIcon::fromTheme(QStringLiteral("arrow-up")), i18n("Show Uploads"));
        show_uploads->setToolTip(i18n("Show all uploads"));
        show_uploads->setCheckable(true);
        connect(show_uploads, &QAction::toggled, this, &QueueManagerWidget::showUploads);

        show_not_queued = toolbar->addAction(QIcon::fromTheme(QStringLiteral("kt-queue-manager")), i18n("Show Not Queued"));
        show_not_queued->setToolTip(i18n("Show all not queued torrents"));
        show_not_queued->setCheckable(true);
        connect(show_not_queued, &QAction::toggled, this, &QueueManagerWidget::showNotQueued);

        model = new QueueManagerModel(qman, this);
        view->setModel(model);
        view->setRootIsDecorated(false);
        view->setAlternatingRowColors(true);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSortingEnabled(false);
        view->setDragDropMode(QAbstractItemView::InternalMove);
        view->setDragEnabled(true);
        view->setAcceptDrops(true);
        view->setDropIndicatorShown(true);
        view->setAutoScroll(true);
        view->setSelectionMode(QAbstractItemView::ContiguousSelection);

        connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        updateButtons();
    }


    QueueManagerWidget::~QueueManagerWidget()
    {}

    void QueueManagerWidget::onTorrentAdded(bt::TorrentInterface* tc)
    {
        model->onTorrentAdded(tc);
    }

    void QueueManagerWidget::onTorrentRemoved(bt::TorrentInterface* tc)
    {
        model->onTorrentRemoved(tc);
    }

    void QueueManagerWidget::moveUpClicked()
    {
        QModelIndexList sel = view->selectionModel()->selectedRows();
        QList<int> rows;
        foreach (const QModelIndex& idx, sel)
            rows.append(idx.row());

        if (rows.isEmpty() || rows.front() == 0)
            return;

        model->moveUp(rows.front(), rows.count());

        QItemSelection nsel;
        int cols = model->columnCount(QModelIndex());
        QModelIndex top_left = model->index(rows.front() - 1, 0);
        QModelIndex bottom_right = model->index(rows.back() - 1, cols - 1);
        nsel.select(top_left, bottom_right);
        view->selectionModel()->select(nsel, QItemSelectionModel::Select);
        if (!indexVisible(top_left))
            view->scrollTo(top_left, QAbstractItemView::PositionAtCenter);

        updateButtons();
    }

    void QueueManagerWidget::moveDownClicked()
    {
        QModelIndexList sel = view->selectionModel()->selectedRows();
        QList<int> rows;
        foreach (const QModelIndex& idx, sel)
            rows.append(idx.row());

        int rowcount = model->rowCount(QModelIndex());
        if (rows.isEmpty() || rows.back() == rowcount - 1)
            return;

        model->moveDown(rows.front(), rows.count());

        QItemSelection nsel;
        int cols = model->columnCount(QModelIndex());
        QModelIndex top_left = model->index(rows.front() + 1, 0);
        QModelIndex bottom_right = model->index(rows.back() + 1, cols - 1);
        nsel.select(top_left, bottom_right);
        view->selectionModel()->select(nsel, QItemSelectionModel::Select);
        if (!indexVisible(top_left))
            view->scrollTo(top_left, QAbstractItemView::PositionAtCenter);

        updateButtons();
    }

    void QueueManagerWidget::moveTopClicked()
    {
        QModelIndexList sel = view->selectionModel()->selectedRows();
        QList<int> rows;
        foreach (const QModelIndex& idx, sel)
            rows.append(idx.row());

        if (rows.isEmpty() || rows.front() == 0)
            return;

        model->moveTop(rows.front(), rows.count());

        QItemSelection nsel;
        int cols = model->columnCount(QModelIndex());
        nsel.select(model->index(0, 0), model->index(rows.count() - 1, cols - 1));
        view->selectionModel()->select(nsel, QItemSelectionModel::Select);
        view->scrollToTop();

        updateButtons();
    }

    void QueueManagerWidget::moveBottomClicked()
    {
        QModelIndexList sel = view->selectionModel()->selectedRows();
        QList<int> rows;
        foreach (const QModelIndex& idx, sel)
            rows.append(idx.row());

        int rowcount = model->rowCount(QModelIndex());
        if (rows.isEmpty() || rows.back() == rowcount - 1)
            return;

        model->moveBottom(rows.front(), rows.count());

        QItemSelection nsel;
        int cols = model->columnCount(QModelIndex());
        nsel.select(model->index(rowcount - rows.count(), 0), model->index(rowcount - 1, cols - 1));
        view->selectionModel()->select(nsel, QItemSelectionModel::Select);
        view->scrollToBottom();

        updateButtons();
    }

    void QueueManagerWidget::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("QueueManagerWidget");
        QByteArray s = view->header()->saveState();
        g.writeEntry("view_state", s.toBase64());
        g.writeEntry("search_text", search->text());
        g.writeEntry("search_bar_visible", show_search->isChecked());
        g.writeEntry("show_uploads", show_uploads->isChecked());
        g.writeEntry("show_downloads", show_downloads->isChecked());
        g.writeEntry("show_not_queued", show_not_queued->isChecked());
    }

    void QueueManagerWidget::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("QueueManagerWidget");
        QByteArray s = QByteArray::fromBase64(g.readEntry("view_state", QByteArray()));
        if (!s.isEmpty())
            view->header()->restoreState(s);

        QString st = g.readEntry("search_text", QString());
        if (!st.isEmpty())
            search->setText(st);

        show_search->setChecked(g.readEntry("search_bar_visible", false));
        show_downloads->setChecked(g.readEntry("show_downloads", true));
        show_uploads->setChecked(g.readEntry("show_uploads", true));
        show_not_queued->setChecked(g.readEntry("show_not_queued", true));
    }

    void QueueManagerWidget::update()
    {
        model->update();
    }

    void QueueManagerWidget::searchTextChanged(const QString& t)
    {
        QModelIndex idx = model->find(t);
        if (idx.isValid())
        {
            view->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        }
    }

    void QueueManagerWidget::showSearch(bool on)
    {
        search->setVisible(on);
    }

    void QueueManagerWidget::showDownloads(bool on)
    {
        model->setShowDownloads(on);
    }

    void QueueManagerWidget::showUploads(bool on)
    {
        model->setShowUploads(on);
    }

    void QueueManagerWidget::showNotQueued(bool on)
    {
        model->setShowNotQueued(on);
    }

    bool QueueManagerWidget::indexVisible(const QModelIndex& idx)
    {
        QRect r = view->visualRect(idx);
        return view->viewport()->rect().contains(r);
    }

    void QueueManagerWidget::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
        Q_UNUSED(selected);
        Q_UNUSED(deselected);
        updateButtons();
    }

    void QueueManagerWidget::updateButtons()
    {
        QModelIndexList idx = view->selectionModel()->selectedRows();
        if (idx.count() == 0)
        {
            move_top->setEnabled(false);
            move_up->setEnabled(false);
            move_down->setEnabled(false);
            move_bottom->setEnabled(false);
        }
        else
        {
            move_top->setEnabled(idx.front().row() != 0);
            move_up->setEnabled(idx.front().row() != 0);

            int rows = model->rowCount(QModelIndex());
            move_down->setEnabled(idx.back().row() != rows - 1);
            move_bottom->setEnabled(idx.back().row() != rows - 1);
        }
    }

}

