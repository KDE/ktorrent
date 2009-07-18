/***************************************************************************
 *   Copyright (C) 2009 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#ifndef KT_VIEWDELEGATE_H
#define KT_VIEWDELEGATE_H

#include <QMap>
#include <QStyledItemDelegate>

namespace bt
{
	class TorrentInterface;
}

namespace kt 
{
	class ViewModel;
	class View;
	class Core;
	class ScanExtender;

	/**
		Item delegate which keeps track of of ScanExtenders
	*/
	class ViewDelegate : public QStyledItemDelegate
	{
		Q_OBJECT
	public:
		ViewDelegate(Core* core,ViewModel* model,View* parent);
		virtual ~ViewDelegate();
		
		/**
			Create a ScanExtender and start checking data.
		*/
		void checkData(bt::TorrentInterface* tc);
		
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		
		/**
		* Close all extenders and delete all extender widgets.
		*/
		void contractAll();
		
		/// Is an extender being shown for a torrent
		bool extended(bt::TorrentInterface* tc) const;
		
		/// Hide the extender for a torrent
		void hideExtender(bt::TorrentInterface* tc);
		
	private slots:
		void closeExtender();
		void torrentRemoved(bt::TorrentInterface* tc);
		
	private:
		QSize maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QRect extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void scheduleUpdateViewLayout();
		
	private:
		ViewModel* model;
		QMap<bt::TorrentInterface*,ScanExtender*> extenders;
		
		typedef QMap<bt::TorrentInterface*,ScanExtender*>::iterator ExtItr;
	};

}

#endif // KT_VIEWDELEGATE_H
