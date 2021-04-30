/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAction>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KToolBar>

#include "mediamodel.h"
#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"
#include "mediaview.h"
#include <util/log.h>

using namespace bt;

namespace kt
{
MediaViewFilter::MediaViewFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
    , show_incomplete(false)
{
}

MediaViewFilter::~MediaViewFilter()
{
}

void MediaViewFilter::setShowIncomplete(bool on)
{
    show_incomplete = on;
    invalidateFilter();
}

bool MediaViewFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (show_incomplete)
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    MediaModel *model = (MediaModel *)sourceModel();
    MediaFileRef ref = model->fileForIndex(model->index(source_row));
    MediaFile::Ptr file = ref.mediaFile();
    if (file->fullyAvailable())
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    else
        return false;
}

void MediaViewFilter::refresh()
{
    invalidateFilter();
}

MediaView::MediaView(MediaModel *model, QWidget *parent)
    : QWidget(parent)
    , model(model)
{
    filter = new MediaViewFilter(this);
    filter->setSourceModel(model);
    filter->setFilterRole(Qt::DisplayRole);
    filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filter->setSortRole(Qt::UserRole + 1);
    filter->sort(0, Qt::DescendingOrder);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(0);
    hbox->setMargin(0);

    tool_bar = new KToolBar(this);
    hbox->addWidget(tool_bar);

    show_incomplete = tool_bar->addAction(QIcon::fromTheme(QStringLiteral("task-ongoing")), i18n("Show incomplete files"));
    show_incomplete->setCheckable(true);
    show_incomplete->setChecked(false);
    connect(show_incomplete, &QAction::toggled, this, &MediaView::showIncompleteChanged);

    refresh = tool_bar->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Refresh"), filter, &MediaViewFilter::refresh);
    refresh->setToolTip(i18n("Refresh media files"));

    search_box = new QLineEdit(this);
    search_box->setClearButtonEnabled(true);
    search_box->setPlaceholderText(i18n("Search media files"));
    connect(search_box, &QLineEdit::textChanged, filter, &MediaViewFilter::setFilterFixedString);
    hbox->addWidget(search_box);

    layout->addLayout(hbox);

    media_tree = new QListView(this);
    media_tree->setModel(filter);
    media_tree->setDragEnabled(true);
    media_tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    media_tree->setAlternatingRowColors(true);
    layout->addWidget(media_tree);

    connect(media_tree, &QListView::doubleClicked, this, &MediaView::onDoubleClicked);
}

MediaView::~MediaView()
{
}

void MediaView::onDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QModelIndex idx = filter->mapToSource(index);
    if (!idx.isValid())
        return;

    doubleClicked(model->fileForIndex(idx));
}

void MediaView::showIncompleteChanged(bool on)
{
    filter->setShowIncomplete(on);
}

void MediaView::loadState(KSharedConfig::Ptr cfg)
{
    KConfigGroup g = cfg->group("MediaView");
    show_incomplete->setChecked(g.readEntry("show_incomplete", false));
    search_box->setText(g.readEntry("search_text", QString()));
}

void MediaView::saveState(KSharedConfig::Ptr cfg)
{
    KConfigGroup g = cfg->group("MediaView");
    g.writeEntry("show_incomplete", show_incomplete->isChecked());
    g.writeEntry("search_text", search_box->text());
}

}
