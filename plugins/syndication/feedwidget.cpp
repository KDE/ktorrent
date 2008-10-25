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
#include "feed.h"
#include "feedwidget.h"
#include "feedwidgetmodel.h"
#include "managefiltersdlg.h"
#include "filterlist.h"
#include "syndicationplugin.h"

namespace kt
{

	FeedWidget::FeedWidget(Feed* feed,FilterList* filters,SyndicationPlugin* plugin,QWidget* parent)
			: QWidget(parent),feed(feed),filters(filters),plugin(plugin)
	{
		setupUi(this);
		connect(feed,SIGNAL(updated()),this,SLOT(updated()));
		connect(m_download,SIGNAL(clicked()),this,SLOT(downloadClicked()));
		connect(m_refresh,SIGNAL(clicked()),this,SLOT(refreshClicked()));
		connect(m_filters,SIGNAL(clicked()),this,SLOT(filtersClicked()));
		
		model = new FeedWidgetModel(feed,this);
		m_item_list->setModel(model);
		m_item_list->setAlternatingRowColors(true);
		m_item_list->setSelectionMode(QAbstractItemView::ContiguousSelection);
		QHeaderView* hv = m_item_list->header();
		hv->setResizeMode(QHeaderView::ResizeToContents);
		connect(m_item_list->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
				this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
		m_download->setEnabled(false);
		m_url->setText(QString("<b>%1</b>").arg(feed->feedUrl().prettyUrl()));
		updated();
	}


	FeedWidget::~FeedWidget()
	{
	}

	void FeedWidget::downloadClicked()
	{
		QModelIndexList sel = m_item_list->selectionModel()->selectedRows();
		foreach (const QModelIndex & idx,sel)
		{
			Syndication::ItemPtr ptr = model->itemForIndex(idx);
			if (ptr.get())
			{
				QString url = TorrentUrlFromItem(ptr);
				if (!url.isEmpty())
					downloadLink(KUrl(url));
				else
					downloadLink(KUrl(ptr->link()));
			}
		}
	}
	
	void FeedWidget::refreshClicked()
	{
		feed->refresh();
	}
	
	void FeedWidget::filtersClicked()
	{
		ManageFiltersDlg dlg(feed,filters,plugin,this);
		if (dlg.exec() == QDialog::Accepted)
		{
			feed->save();
			feed->runFilters();
		}
	}
	
	void FeedWidget::selectionChanged(const QItemSelection& sel, const QItemSelection& prev)
	{
		Q_UNUSED(prev);
		m_download->setEnabled(sel.count() > 0);
	}
	
	void FeedWidget::updated()
	{
		switch (feed->feedStatus())
		{
			case Feed::OK:
				m_status->setText(i18n("<b>OK</b>"));
				break;
			case Feed::UNLOADED:
				m_status->setText(i18n("<b>Not Loaded</b>"));
				break;
			case Feed::FAILED_TO_DOWNLOAD:
				m_status->setText(i18n("<b>Download Failed</b>"));
				break;
			case Feed::DOWNLOADING:
				m_status->setText(i18n("<b>Downloading</b>"));
				break;
		}
		updateCaption(this,feed->title());
	}
}
