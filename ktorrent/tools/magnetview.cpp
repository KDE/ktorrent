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


#include "magnetview.h"
#include "magnetmodel.h"
#include <KMenu>
#include <KLocalizedString>
#include <KIcon>
#include <KConfigGroup>
#include <QHeaderView>
#include <QKeyEvent>

namespace kt
{
	
	MagnetView::MagnetView(MagnetModel* magnet_model,QWidget* parent): QTreeView(parent),magnet_model(magnet_model)
	{
		setAllColumnsShowFocus(true);
		setRootIsDecorated(false);
		setContextMenuPolicy(Qt::CustomContextMenu);
		setModel(magnet_model);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
		connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
		
		menu = new KMenu(this);
		start = menu->addAction(KIcon("kt-start"),i18n("Start Magnet"),this,SLOT(startMagnetDownload()));
		stop = menu->addAction(KIcon("kt-stop"),i18n("Stop Magnet"),this,SLOT(stopMagnetDownload()));
		menu->addSeparator();
		remove = menu->addAction(KIcon("kt-remove"),i18n("Remove Magnet"),this,SLOT(removeMagnetDownload()));
	}

	MagnetView::~MagnetView()
	{
	}
	
	void MagnetView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MagnetView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
		{
			QHeaderView* v = header();
			v->restoreState(s);
		}
	}

	void MagnetView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MagnetView");
		g.writeEntry("state",header()->saveState().toBase64());
	}

	void MagnetView::showContextMenu(QPoint p)
	{
		QModelIndexList idx_list = selectionModel()->selectedRows();
		
		start->setEnabled(false);
		stop->setEnabled(false);
		remove->setEnabled(idx_list.count() > 0);
		
		foreach (const QModelIndex & idx,idx_list)
		{
			kt::MagnetDownloader* md = (kt::MagnetDownloader*)idx.internalPointer();
			if (md->running())
				stop->setEnabled(true);
			else
				start->setEnabled(true);
		}
		menu->popup(viewport()->mapToGlobal(p));
	}

	void MagnetView::removeMagnetDownload()
	{
		QList<kt::MagnetDownloader*> mdl;
		QModelIndexList idx_list = selectionModel()->selectedRows();
		foreach (const QModelIndex & idx,idx_list)
		{
			mdl.append((kt::MagnetDownloader*)idx.internalPointer());
		}
		
		foreach (kt::MagnetDownloader* md,mdl)
			magnet_model->removeMagnetDownloader(md);
	}

	void MagnetView::startMagnetDownload()
	{
		QModelIndexList idx_list = selectionModel()->selectedRows();
		foreach (const QModelIndex & idx,idx_list)
		{
			magnet_model->start(idx);
		}
	}

	void MagnetView::stopMagnetDownload()
	{
		QModelIndexList idx_list = selectionModel()->selectedRows();
		foreach (const QModelIndex & idx,idx_list)
		{
			magnet_model->stop(idx);
		}
	}

	void MagnetView::keyPressEvent(QKeyEvent* event)
	{
		if(event->key() == Qt::Key_Delete)
		{
			removeMagnetDownload();
			event->accept();
		}
		else
			QTreeView::keyPressEvent(event);
	}


}

