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
#include <kglobal.h>
#include <kconfig.h>
#include <interfaces/torrentinterface.h>
#include "downloadorderdialog.h"
#include "downloadordermanager.h"
#include "downloadorderplugin.h"
#include "downloadordermodel.h"

namespace kt
{

	DownloadOrderDialog::DownloadOrderDialog(DownloadOrderPlugin* plugin,bt::TorrentInterface* tor,QWidget* parent)
			: KDialog(parent),tor(tor),plugin(plugin)
	{
		setupUi(mainWidget());
		setButtons(KDialog::Ok | KDialog::Cancel);
		connect(this,SIGNAL(okClicked()),this,SLOT(commitDownloadOrder()));
		setCaption(i18n("File Download Order"));
		m_top_label->setText(i18n("File download order for <b>%1</b>:",tor->getDisplayName()));
		
		DownloadOrderManager* dom = plugin->manager(tor);
		m_custom_order_enabled->setChecked(dom != 0);
		m_order->setEnabled(dom != 0);
		m_move_up->setEnabled(dom != 0);
		m_move_down->setEnabled(dom != 0);
		
		m_move_up->setIcon(KIcon("go-up"));
		connect(m_move_up,SIGNAL(clicked()),this,SLOT(moveUp()));
		m_move_down->setIcon(KIcon("go-down"));
		connect(m_move_down,SIGNAL(clicked()),this,SLOT(moveDown()));
		
		m_order->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_order->setDragEnabled(true);
		m_order->setAcceptDrops(true);
		m_order->setDropIndicatorShown(true);
		m_order->setDragDropMode(QAbstractItemView::InternalMove);
		
		model = new DownloadOrderModel(tor,this);
		if (dom)
			model->initOrder(dom->downloadOrder());
		m_order->setModel(model);
		
		QSize s = KGlobal::config()->group("DownloadOrderDialog").readEntry("size",size());
		resize(s);
	}


	DownloadOrderDialog::~DownloadOrderDialog()
	{
		KGlobal::config()->group("DownloadOrderDialog").writeEntry("size",size());
	}

	void DownloadOrderDialog::commitDownloadOrder()
	{
		if (m_custom_order_enabled->isChecked())
		{
			DownloadOrderManager* dom = plugin->manager(tor);
			if (!dom)
			{
				dom = plugin->createManager(tor);
				connect(tor,SIGNAL(chunkDownloaded(bt::TorrentInterface*, bt::Uint32)),
						dom,SLOT(chunkDownloaded(bt::TorrentInterface*, bt::Uint32)));
			}
			
			dom->setDownloadOrder(model->downloadOrder());
			dom->save();
			dom->update();
		}
		else
		{
			DownloadOrderManager* dom = plugin->manager(tor);
			if (dom)
			{
				dom->disable();
				plugin->destroyManager(tor);
			}
		}
		accept();
	}
	
	void DownloadOrderDialog::moveUp()
	{
		QModelIndex idx = m_order->selectionModel()->currentIndex();
		model->moveUp(idx);
		if (idx.row() > 0)
			m_order->selectionModel()->setCurrentIndex(model->index(idx.row() - 1,0),QItemSelectionModel::ClearAndSelect);
	}
	
	void DownloadOrderDialog::moveDown()
	{
		QModelIndex idx = m_order->selectionModel()->currentIndex();
		model->moveDown(idx);
		if (idx.row() < (int)tor->getNumFiles() - 1)
			m_order->selectionModel()->setCurrentIndex(model->index(idx.row() + 1,0),QItemSelectionModel::ClearAndSelect);
	}
}
