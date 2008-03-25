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
#include "webseedstab.h"
#include "webseedsmodel.h"

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
		//m_webseed_list->setShowSortIndicator(true);
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
	}
		
	void WebSeedsTab::addWebSeed()
	{
	}
		
	void WebSeedsTab::removeWebSeed()
	{
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
