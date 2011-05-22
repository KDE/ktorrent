/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <KLineEdit>
#include <KLocale>
#include <util/log.h>
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "mediaplayerpluginsettings.h"



using namespace bt;

namespace kt
{
	MediaViewFilter::MediaViewFilter(QObject* parent) 
		: QSortFilterProxyModel(parent),
		show_incomplete(false)
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

	bool MediaViewFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
	{
		if (show_incomplete)
			return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		
		MediaModel* model = (MediaModel*)sourceModel();
		MediaFileRef ref = model->fileForIndex(model->index(source_row));
		MediaFile::Ptr file = ref.mediaFile();
		if (file->fullyAvailable())
			return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		else
			return false;
	}

	

	MediaView::MediaView(MediaModel* model,QWidget* parent)
			: QWidget(parent),model(model)
	{
		filter = new MediaViewFilter(this);
		filter->setSourceModel(model);
		filter->setFilterRole(Qt::DisplayRole);
		filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
		filter->setSortRole(Qt::UserRole + 1);
		filter->sort(0,Qt::DescendingOrder);
		
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		search_box = new KLineEdit(this);
		search_box->setClearButtonShown(true);
		search_box->setClickMessage(i18n("Search media files"));
		connect(search_box,SIGNAL(textChanged(QString)),filter,SLOT(setFilterFixedString(QString)));
		layout->addWidget(search_box);
		
		media_tree = new QListView(this);
		media_tree->setModel(filter);
		media_tree->setDragEnabled(true);
		media_tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
		media_tree->setAlternatingRowColors(true);
		layout->addWidget(media_tree);
		
		show_incomplete = new QCheckBox(i18n("Show incomplete files"),this);
		show_incomplete->setChecked(false);
		connect(show_incomplete,SIGNAL(stateChanged(int)),this,SLOT(showIncompleteChanged(int)));
		layout->addWidget(show_incomplete);
		
		connect(media_tree,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(onDoubleClicked(QModelIndex)));
	}


	MediaView::~MediaView()
	{
	}
	
	void MediaView::onDoubleClicked(const QModelIndex& index)
	{
		if (!index.isValid())
			return;
		
		QModelIndex idx = filter->mapToSource(index);
		if (!idx.isValid())
			return;
		
		doubleClicked(model->fileForIndex(idx));
	}
	
	void MediaView::showIncompleteChanged(int state)
	{
		filter->setShowIncomplete(state == Qt::Checked);
	}

	void MediaView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MediaView");
		show_incomplete->setChecked(g.readEntry("show_incomplete", false));
	}

	void MediaView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MediaView");
		g.writeEntry("show_incomplete", show_incomplete->isChecked());
	}

}
