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

class QVBoxLayout;
namespace bt
{
	class TorrentInterface;
}

namespace kt 
{
	class ViewModel;
	class View;
	class Core;
	
	/**
		Base class for all extender widgets
	*/
	class Extender : public QWidget
	{
		Q_OBJECT
	public:
		Extender(bt::TorrentInterface* tc,QWidget* parent);
		virtual ~Extender();
		
		/// Get the torrent of this extender
		bt::TorrentInterface* torrent() {return tc;}
		
	signals:
		/// Should be emitted by an extender when it wants to close itself
		void closeRequest(Extender* ext);
		
		/// Should be emitted when an extender is resized
		void resized(Extender* ext);
		
	protected:
		bt::TorrentInterface* tc;
	};
	
	/**
		Box which contains all the extenders of a widget
	*/
	class ExtenderBox : public QWidget
	{
	public:
		ExtenderBox(QWidget* widget);
		virtual ~ExtenderBox();
		
		/// Add an Extender
		void add(Extender* ext);
		
		/// Remove an Extender
		void remove(Extender* ext);
		
		/// Clear all extenders
		void clear();
		
		/// Get the number of extenders
		int count() const {return extenders.count();}
		
	private:
		QVBoxLayout* layout;
		QList<Extender*> extenders;
	};

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
			Extend a torrent with a widget
		*/
		void extend(bt::TorrentInterface* tc,Extender* widget);
		
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual void updateEditorGeometry(QWidget* editor,const QStyleOptionViewItem & option,const QModelIndex & index) const;
		
		/**
		* Close all extenders and delete all extender widgets.
		*/
		void contractAll();
		
		/// Is an extender being shown for a torrent
		bool extended(bt::TorrentInterface* tc) const;
		
		/// Hide the extender for a torrent
		void hideExtender(bt::TorrentInterface* tc);
		
		/// Does the delegate have extenders
		bool hasExtenders() const {return !extenders.isEmpty();}
		
	public slots:
		/// Close all the extenders of a torrent
		void closeExtenders(bt::TorrentInterface* tc);
		void closeExtender(bt::TorrentInterface* tc,Extender* ext);
		
	private slots:
		void torrentRemoved(bt::TorrentInterface* tc);
		void closeRequested(Extender* ext);
		void resized(Extender* ext);
		
	private:
		QSize maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QRect extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void scheduleUpdateViewLayout();
		void paintProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void normalPaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		
	private:
		ViewModel* model;
		QMap<bt::TorrentInterface*,ExtenderBox*> extenders;
		
		typedef QMap<bt::TorrentInterface*,ExtenderBox*>::iterator ExtItr;
		typedef QMap<bt::TorrentInterface*,ExtenderBox*>::const_iterator ExtCItr;
	};

}

#endif // KT_VIEWDELEGATE_H
