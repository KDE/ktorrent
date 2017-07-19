/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
*   joris.guisson@gmail.com                                               *
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

#include "playlistwidget.h"

#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QIcon>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KFileWidget>
#include <KRecentDirs>
#include <KToolBar>

#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"
#include "playlist.h"


namespace kt
{
    PlayListWidget::PlayListWidget(kt::MediaFileCollection* collection, kt::MediaPlayer* player, QWidget* parent)
        : QWidget(parent),
          player(player),
          menu(nullptr),
          collection(collection)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);


        QAction* remove_action = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove"), this);
        connect(remove_action, &QAction::triggered, this, &PlayListWidget::removeFiles);
        QAction* add_action = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("Add Media"), this);
        connect(add_action, &QAction::triggered, this, &PlayListWidget::addMedia);
        QAction* clear_action = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18n("Clear Playlist"), this);
        connect(clear_action, &QAction::triggered, this, &PlayListWidget::clearPlayList);

        tool_bar = new QToolBar(this);
        tool_bar->addAction(add_action);
        tool_bar->addAction(remove_action);
        tool_bar->addAction(clear_action);
        random_mode = new QCheckBox(i18n("Random play order"), tool_bar);
        connect(random_mode, &QCheckBox::toggled, this, &PlayListWidget::randomModeActivated);
        tool_bar->addWidget(random_mode);
        layout->addWidget(tool_bar);

        play_list = new PlayList(collection, player, this);
        connect(play_list, &PlayList::itemsDropped, this, &PlayListWidget::onItemsDropped);
        proxy_model = new QSortFilterProxyModel(this);
        proxy_model->setSourceModel(play_list);
        proxy_model->setSortRole(Qt::UserRole);

        view = new QTreeView(this);
        view->setModel(proxy_model);
        view->setDragEnabled(true);
        view->setDropIndicatorShown(true);
        view->setAcceptDrops(true);
        view->setAlternatingRowColors(true);
        view->setRootIsDecorated(false);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setSortingEnabled(true);
        layout->addWidget(view);
        connect(view, &QTreeView::customContextMenuRequested, this, &PlayListWidget::showContextMenu);

        connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& , const QItemSelection&)),
                this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
        connect(view, &QTreeView::doubleClicked, this, static_cast<void (PlayListWidget::*)(const QModelIndex&)>(&PlayListWidget::doubleClicked));

        menu = new QMenu(this);
        menu->addAction(remove_action);
        menu->addSeparator();
        menu->addAction(add_action);
        menu->addAction(clear_action);
    }

    PlayListWidget::~PlayListWidget()
    {
    }

    QModelIndex PlayListWidget::selectedItem() const
    {
        QModelIndexList rows = view->selectionModel()->selectedRows();
        if (rows.count() > 0)
            return proxy_model->mapToSource(rows.front());
        else
            return QModelIndex();
    }

    void PlayListWidget::onSelectionChanged(const QItemSelection& s, const QItemSelection& d)
    {
        Q_UNUSED(d);
        QModelIndexList idx = s.indexes();
        if (idx.count() > 0)
            fileSelected(fileForIndex(idx.front()));
        else
            fileSelected(MediaFileRef());
    }

    QModelIndex PlayListWidget::play()
    {
        QModelIndex pidx = view->currentIndex();
        QModelIndex idx = proxy_model->mapToSource(pidx);
        MediaFileRef file = play_list->fileForIndex(idx);
        if (!file.path().isEmpty())
        {
            player->play(file);
        }
        return pidx;
    }

    void PlayListWidget::doubleClicked(const QModelIndex& index)
    {
        MediaFileRef file = play_list->fileForIndex(proxy_model->mapToSource(index));
        if (!file.path().isEmpty())
            doubleClicked(file);
    }

    void PlayListWidget::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("PlayListWidget");
        QHeaderView* v = view->header();
        g.writeEntry("play_list_state", v->saveState());
        g.writeEntry("random_mode", random_mode->isChecked());
    }

    void PlayListWidget::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("PlayListWidget");
        QByteArray d = g.readEntry("play_list_state", QByteArray());
        if (!d.isEmpty())
            view->header()->restoreState(d);

        view->header()->setSortIndicatorShown(true);
        random_mode->setChecked(g.readEntry("random_mode", false));
    }

    void PlayListWidget::showContextMenu(QPoint pos)
    {
        menu->popup(view->viewport()->mapToGlobal(pos));
    }

    void PlayListWidget::clearPlayList()
    {
        play_list->clear();
        enableNext(false);
        fileSelected(MediaFileRef());
    }

    void PlayListWidget::addMedia()
    {
        QString recentDirClass;
        QStringList files = QFileDialog::getOpenFileNames(this, QString(),
                                                         KFileWidget::getStartUrl(QUrl(QStringLiteral("kfiledialog:///add_media")), recentDirClass).toLocalFile());

        if (files.isEmpty())
            return;

        if (!recentDirClass.isEmpty())
            KRecentDirs::add(recentDirClass, QFileInfo(files.first()).absolutePath());

        foreach (const QString& file, files)
            play_list->addFile(collection->find(file));

        enableNext(play_list->rowCount() > 0);
    }

    void PlayListWidget::removeFiles()
    {
        QList<MediaFileRef> files;
        QModelIndexList indexes = view->selectionModel()->selectedRows();
        foreach (const QModelIndex& idx, indexes)
            files.append(play_list->fileForIndex(idx));

        foreach (const MediaFileRef& f, files)
            play_list->removeFile(f);

        enableNext(play_list->rowCount() > 0);
    }

    void PlayListWidget::onItemsDropped()
    {
        enableNext(play_list->rowCount() > 0);
    }

    QModelIndex PlayListWidget::next(const QModelIndex& idx, bool random) const
    {
        if (play_list->rowCount() == 0)
            return QModelIndex();

        if (!idx.isValid())
        {
            if (!random)
            {
                return proxy_model->index(0, 0, QModelIndex());
            }
            else
            {
                return randomNext(QModelIndex());
            }
        }
        else if (!random)
        {
            return next(idx);
        }
        else
        {
            return randomNext(idx);
        }
    }

    QModelIndex PlayListWidget::next(const QModelIndex& idx) const
    {
        if (idx.isValid())
            return idx.sibling(idx.row() + 1, 0); // take a look at the next sibling
        else
            return play_list->index(0, 0);
    }

    QModelIndex PlayListWidget::randomNext(const QModelIndex& idx) const
    {
        int count = play_list->rowCount();
        if (count <= 1)
            return QModelIndex();

        int r = qrand() % count;
        while (r == idx.row())
            r = qrand() % count;

        return proxy_model->index(r, 0, QModelIndex());
    }


    QString PlayListWidget::fileForIndex(const QModelIndex& index) const
    {
        return play_list->fileForIndex(proxy_model->mapToSource(index)).path();
    }


    QModelIndex PlayListWidget::indexForFile(const QString& file) const
    {
        int count = proxy_model->rowCount();
        for (int i = 0; i < count; i++)
        {
            QModelIndex idx = proxy_model->index(i, 0);
            if (fileForIndex(idx) == file)
                return idx;
        }

        return QModelIndex();
    }


}
