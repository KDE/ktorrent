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
#include "viewdelegate.h"
#include "viewmodel.h"
#include "core.h"
#include "view.h"

namespace kt
{
	
	ViewDelegate::ViewDelegate(Core* core,ViewModel* model,View* parent): QStyledItemDelegate(parent),model(model)
	{
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(torrentRemoved(bt::TorrentInterface*)));
	}

	ViewDelegate::~ViewDelegate()
	{
		contractAll();
	}
	
	
	void ViewDelegate::extend(bt::TorrentInterface* tc, QWidget* widget)
	{
		ExtItr itr = extenders.find(tc);
		QWidget* ext = itr == extenders.end() ? 0 : itr.value();
		if (!ext)
		{
			ext = widget;
			extenders.insert(tc,ext);
			QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
			if (aiv) 
				ext->setParent(aiv->viewport());
			
			scheduleUpdateViewLayout();
		}
	}
	
	void ViewDelegate::closeExtender(bt::TorrentInterface* tc)
	{
		ExtItr itr = extenders.find(tc);
		QWidget* ext = itr == extenders.end() ? 0 : itr.value();
		if (ext)
		{
			extenders.remove(tc);
			ext->hide();
			ext->deleteLater();
			scheduleUpdateViewLayout();
		}
	}

	void ViewDelegate::torrentRemoved(bt::TorrentInterface* tc)
	{
		ExtItr itr = extenders.find(tc);
		if (itr != extenders.end())
			extenders.erase(itr);
	}
	
	
	QSize ViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize ret;
		
		if (!extenders.isEmpty()) 
			ret = maybeExtendedSize(option, index);
		else
			ret = QStyledItemDelegate::sizeHint(option, index);
		
		return ret;
	}
	
	QSize ViewDelegate::maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		bt::TorrentInterface* tc = model->torrentFromIndex(index);
		QSize size(QStyledItemDelegate::sizeHint(option, index));
		if (!tc)
			return size;
		
		ExtCItr itr = extenders.find(tc);
		const QWidget* ext = itr == extenders.end() ? 0 : itr.value();
		if (!ext)
			return size;
		
		//add extender height to maximum height of any column in our row
		int item_height = size.height();
		int row = index.row();
		int this_column = index.column();
		
		//this is quite slow, but Qt is smart about when to call sizeHint().
		for (int column = 0; model->columnCount() < column; column++) 
		{
			if (column == this_column) 
				continue;
			
			QModelIndex neighborIndex(index.sibling(row, column));
			if (neighborIndex.isValid()) 
				item_height = qMax(item_height, QStyledItemDelegate::sizeHint(option, neighborIndex).height());
		}
		
		//we only want to reserve vertical space, the horizontal extender layout is our private business.
		size.rheight() = item_height + ext->sizeHint().height();
		return size;
	}

	
	void ViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QStyleOptionViewItemV4 indicatorOption(option);
		initStyleOption(&indicatorOption, index);
		if (index.column() == 0) 
			indicatorOption.viewItemPosition = QStyleOptionViewItemV4::Beginning;
		else if (index.column() == index.model()->columnCount() - 1) 
			indicatorOption.viewItemPosition = QStyleOptionViewItemV4::End;
		else 
			indicatorOption.viewItemPosition = QStyleOptionViewItemV4::Middle;
		
		QStyleOptionViewItemV4 itemOption(option);
		initStyleOption(&itemOption, index);
		if (index.column() == 0) 
			itemOption.viewItemPosition = QStyleOptionViewItemV4::Beginning;
		else if (index.column() == index.model()->columnCount() - 1)
			itemOption.viewItemPosition = QStyleOptionViewItemV4::End;
		else 
			itemOption.viewItemPosition = QStyleOptionViewItemV4::Middle;
		
		
		bt::TorrentInterface* tc = model->torrentFromIndex(index);
		if (!tc || !extenders.contains(tc))
		{
			QStyledItemDelegate::paint(painter, itemOption, index);
			return;
		}
		
		QWidget* extender = extenders[tc];
		int extenderHeight = extender->sizeHint().height();
		
		
		//an extender is present - make two rectangles: one to paint the original item, one for the extender
		QStyleOptionViewItemV4 extOption(option);
		initStyleOption(&extOption, index);
		extOption.rect = extenderRect(extender, option, index);
		extender->setGeometry(extOption.rect);
		//if we show it before, it will briefly flash in the wrong location.
		//the downside is, of course, that an api user effectively can't hide it.
		extender->show();
		
		indicatorOption.rect.setHeight(option.rect.height() - extenderHeight);
		itemOption.rect.setHeight(option.rect.height() - extenderHeight);
		//tricky:make sure that the modified options' rect really has the
		//same height as the unchanged option.rect if no extender is present
		//(seems to work OK)
		QStyledItemDelegate::paint(painter, itemOption, index);
	}
	
	QRect ViewDelegate::extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QRect rect(option.rect);
		rect.setTop(rect.bottom() + 1 - extender->sizeHint().height());
		
		rect.setLeft(0);
		if (QTreeView *tv = qobject_cast<QTreeView *>(parent())) 
		{
			int steps = 0;
			for (QModelIndex idx(index.parent()); idx.isValid(); idx = idx.parent()) 
				steps++;
			
			if (tv->rootIsDecorated()) 
				steps++;
			
			rect.setLeft(steps * tv->indentation());
		}
		
		QAbstractScrollArea *container = qobject_cast<QAbstractScrollArea *>(parent());
		rect.setRight(container->viewport()->width() - 1);
		return rect;
	}


	void ViewDelegate::scheduleUpdateViewLayout()
	{
		QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
		//prevent crashes during destruction of the view
		if (aiv) 
		{
			//dirty hack to call aiv's protected scheduleDelayedItemsLayout()
			aiv->setRootIndex(aiv->rootIndex());
		}
	}


	void ViewDelegate::contractAll()
	{
		for (ExtItr i = extenders.begin();i != extenders.end();i++)
		{
			i.value()->hide();
			i.value()->deleteLater();
		}
		extenders.clear();
	}
	
	
	bool ViewDelegate::extended(bt::TorrentInterface* tc) const
	{
		return extenders.contains(tc);
	}
	
	
	void ViewDelegate::hideExtender(bt::TorrentInterface* tc)
	{
		ExtItr i = extenders.find(tc);
		if (i != extenders.end())
			i.value()->hide();
	}



}

