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
#ifndef KTVIEWSELECTIONMODEL_H
#define KTVIEWSELECTIONMODEL_H

#include <QSet>
#include <QItemSelectionModel>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class ViewModel;

	/**
		Custom selection model for View
	*/
	class ViewSelectionModel : public QItemSelectionModel
	{
		Q_OBJECT
	public:
		ViewSelectionModel(ViewModel* vm,QObject* parent);
		virtual ~ViewSelectionModel();
		
		virtual void select(const QModelIndex & index,QItemSelectionModel::SelectionFlags command);
		virtual void select(const QItemSelection & selection,QItemSelectionModel::SelectionFlags command);
		virtual void clear();
		virtual void reset();
		
	public slots:
		/// Called by view whenever the model is sorted
		void sorted();
		
	private:
		ViewModel* vm;
		QSet<bt::TorrentInterface*> selection;
	};

}

#endif
