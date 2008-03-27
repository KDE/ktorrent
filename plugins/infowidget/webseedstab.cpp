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
#include <kmessagebox.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/webseedinterface.h>
#include "webseedstab.h"
#include "webseedsmodel.h"

using namespace bt;

namespace kt
{

	WebSeedsTab::WebSeedsTab(QWidget* parent)
			: QWidget(parent),curr_tc(0)
	{
		setupUi(this);
		connect(m_add,SIGNAL(clicked()),this,SLOT(addWebSeed()));
		connect(m_remove,SIGNAL(clicked()),this,SLOT(removeWebSeed()));
		m_add->setIcon(KIcon("list-add"));
		m_remove->setIcon(KIcon("list-remove"));
		m_add->setEnabled(false);
		m_remove->setEnabled(false);
		m_webseed_list->setEnabled(false);
		model = new WebSeedsModel(this);
		proxy_model = new QSortFilterProxyModel(this);
		proxy_model->setSourceModel(model);
		proxy_model->setSortRole(Qt::UserRole);
		m_webseed_list->setModel(proxy_model);
		m_webseed_list->setSortingEnabled(true);
		
		connect(m_webseed_list->selectionModel(),SIGNAL( selectionChanged ( const QItemSelection & , const QItemSelection &  )),
				this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
		
		connect(m_webseed,SIGNAL(textChanged(const QString &)),this,SLOT(onWebSeedTextChanged(const QString&)));
	}


	WebSeedsTab::~WebSeedsTab()
	{
	}

	void WebSeedsTab::changeTC(bt::TorrentInterface* tc)
	{
		curr_tc = tc;
		model->changeTC(tc);
		m_add->setEnabled(curr_tc != 0);
		m_remove->setEnabled(curr_tc != 0);
		m_webseed_list->setEnabled(curr_tc != 0);
		m_webseed->setEnabled(curr_tc != 0);
		onWebSeedTextChanged(m_webseed->text());
		
		// see if we need to enable or disable the remove button
		if (curr_tc)
			selectionChanged(m_webseed_list->selectionModel()->selectedRows());
	}
		
	void WebSeedsTab::addWebSeed()
	{
		if (!curr_tc)
			return;
		
		KUrl url(m_webseed->text());
		if (curr_tc != 0 && url.isValid() && url.protocol() == "http")
		{
			if (curr_tc->addWebSeed(url))
			{
				model->changeTC(curr_tc);
				m_webseed->clear();
			}
			else
			{
				KMessageBox::error(this,i18n("Cannot add the webseed %1, it is already part of the list of webseeds."));
			}
		}
	}
		
	void WebSeedsTab::removeWebSeed()
	{
		if (!curr_tc)
			return;
		
		QModelIndexList idx_list = m_webseed_list->selectionModel()->selectedRows();
		foreach (QModelIndex idx,idx_list)
		{
			idx = proxy_model->mapToSource(idx);
			const WebSeedInterface* ws = curr_tc->getWebSeed(idx.row());
			if (ws && ws->isUserCreated())
			{
				if (!curr_tc->removeWebSeed(ws->getUrl()))
					KMessageBox::error(this,i18n("Cannot remove webseed %s, it is part of the torrent.",ws->getUrl().prettyUrl()));
			}
		}
		
		model->changeTC(curr_tc);
	}
	
	void WebSeedsTab::selectionChanged(const QModelIndexList & indexes)
	{
		foreach (QModelIndex idx,indexes)
		{
			idx = proxy_model->mapToSource(idx);
			const WebSeedInterface* ws = curr_tc->getWebSeed(idx.row());
			if (ws && ws->isUserCreated())
			{
				m_remove->setEnabled(true);
				return;
			}
		}
		
		m_remove->setEnabled(false);
	}
	
	void WebSeedsTab::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
	{
		Q_UNUSED(deselected);
		if (!curr_tc)
			return;
		
		selectionChanged(selected.indexes());
	}
	
	void WebSeedsTab::onWebSeedTextChanged(const QString & ws)
	{
		KUrl url(ws);
		m_add->setEnabled(curr_tc != 0 && url.isValid() && url.protocol() == "http");
	}
	
	void WebSeedsTab::update()
	{
		if (model->update())
			proxy_model->invalidate();
	}
		
	void WebSeedsTab::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("WebSeedsTab");
		QByteArray s = m_webseed_list->header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void WebSeedsTab::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("WebSeedsTab");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			m_webseed_list->header()->restoreState(s);
	}
}
