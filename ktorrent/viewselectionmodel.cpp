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
#include <util/log.h>
#include "viewselectionmodel.h"
#include "viewmodel.h"

using namespace bt;

namespace kt
{

	ViewSelectionModel::ViewSelectionModel(ViewModel* vm,QObject* parent)
			: QItemSelectionModel(vm,parent),vm(vm)
	{
	}


	ViewSelectionModel::~ViewSelectionModel()
	{
	}

	void ViewSelectionModel::select(const QModelIndex & index,QItemSelectionModel::SelectionFlags command)
	{
		QItemSelection sel(index,index);
		select(sel,command);
	}
	
	void ViewSelectionModel::select(const QItemSelection & sel,QItemSelectionModel::SelectionFlags command)
	{
		if (command == NoUpdate)
			return;
		
		if (command & QItemSelectionModel::Clear)
			selection.clear();
		
		foreach (const QItemSelectionRange r,sel)
		{
			for (int i = r.topLeft().row();i <= r.bottomRight().row();i++)
			{
				bt::TorrentInterface* tor = vm->torrentFromRow(i);
				if (!tor)
					continue;
				
				if (command & QItemSelectionModel::Select)
				{
					selection.insert(tor);
				}
				else if (command & QItemSelectionModel::Deselect)
				{
					selection.remove(tor);
				}
				else if (command & QItemSelectionModel::Toggle)
				{
					if (selection.contains(tor))
						selection.remove(tor);
					else
						selection.insert(tor);
				}
			}
		}
		QItemSelectionModel::select(sel,command);
	}
	
	void ViewSelectionModel::clear()
	{
		selection.clear();
		QItemSelectionModel::clear();
	}
	
	void ViewSelectionModel::reset()
	{
		selection.clear();
		QItemSelectionModel::reset();
	}
	
	void ViewSelectionModel::sorted()
	{
		QItemSelection ns;
		int rows = vm->rowCount(QModelIndex());
		int cols = vm->columnCount(QModelIndex());
		for (int i = 0;i < rows;i++)
		{
			QModelIndex idx = vm->index(i,0,QModelIndex());
			bt::TorrentInterface* tc = vm->torrentFromIndex(idx);
			if (tc && selection.contains(tc))
			{
				ns.select(idx,vm->index(i,cols - 1,QModelIndex()));
			}
		}
		
		select(ns,QItemSelectionModel::ClearAndSelect);
	}
}
