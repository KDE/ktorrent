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
#ifndef KTROUTERMODEL_H
#define KTROUTERMODEL_H

#include <QAbstractTableModel>

namespace kt 
{
	class UPnPRouter;
	
	/**
		Model for all the detected UPnP routers.
	*/
	class RouterModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		RouterModel(QObject* parent);	
		virtual ~RouterModel();
		
		/// Add a router tot the model
		void addRouter(UPnPRouter* r);
		
		/// Get a router given an index
		UPnPRouter* routerForIndex(const QModelIndex & index);
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		
		void emitReset();
		
	private:
		QString ports(const UPnPRouter* r) const;
		QString connections(const UPnPRouter* r) const;
		
	private:
		QList<UPnPRouter*> routers;
	};

}

#endif
