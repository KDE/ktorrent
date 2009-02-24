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
#include <QTimer>
#include <QMouseEvent>
#include <QActionGroup>
#include <QSortFilterProxyModel>
#include <KIcon>
#include <KMenu>
#include <KAction>
#include <KLocale>
#include <interfaces/activity.h>
#include "activitylistwidget.h"
#include "activitylistdelegate.h"
#include "activitylistmodel.h"
#include <kconfiggroup.h>


namespace kt
{
	const int LITTLE_ICON_SIZE = 22;
	const int NORMAL_ICON_SIZE = 32;
	const int BIG_ICON_SIZE = 48;
/*	
	class SortFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		SortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
		{
			setDynamicSortFilter(true);
			sort(0);
		}
		
	protected:
		bool lessThan(const QModelIndex & left,const QModelIndex & right) const
		{
			Kontact::Plugin *leftPlugin = static_cast<Kontact::Plugin*>( left.internalPointer() );
			Kontact::Plugin *rightPlugin = static_cast<Kontact::Plugin*>( right.internalPointer() );
			
			if ( leftPlugin->weight() == rightPlugin->weight() ) {
				return KStringHandler::naturalCompare( leftPlugin->title(), rightPlugin->title() ) < 0;
			}
			
			return leftPlugin->weight() < rightPlugin->weight();
		}
	};
	*/

	
	ActivityListWidget::ActivityListWidget(QWidget* parent)
		: QListView(parent),icon_size(NORMAL_ICON_SIZE),mode(ICONS_AND_TEXT)
	{
		setFocusPolicy(Qt::NoFocus);
		delegate = new ActivityListDelegate(icon_size,this);
		setItemDelegate(delegate);
		model = new ActivityListModel(this);
		setModel(model);
		setViewMode(ListMode);
	
		setViewport(new QWidget(this));
		setVerticalScrollMode(ScrollPerPixel);
		setHorizontalScrollMode(ScrollPerPixel);
		setIconSize(QSize(icon_size, icon_size));
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showConfigMenu(QPoint)));
	
		
		menu = new KMenu(this);
		QActionGroup* icon_size = new QActionGroup(this);
		icon_size->setExclusive(true);
		little_icons = menu->addAction(i18n("Little icons"));
		little_icons->setCheckable(true);
		normal_icons = menu->addAction(i18n("Normal icons"));
		normal_icons->setCheckable(true);
		big_icons = menu->addAction(i18n("Big icons"));
		big_icons->setCheckable(true);
		icon_size->addAction(little_icons);
		icon_size->addAction(normal_icons);
		icon_size->addAction(big_icons);
		normal_icons->setChecked(true);
		menu->addSeparator();
		
		
		QActionGroup* icon_mode = new QActionGroup(this);
		show_icons_only = menu->addAction(i18n("Icons only"));
		show_icons_only->setCheckable(true);
		icon_mode->addAction(show_icons_only);
		
		show_text_only = menu->addAction(i18n("Text only"));
		show_text_only->setCheckable(true);
		icon_mode->addAction(show_text_only);
		
		show_icons_and_text = menu->addAction(i18n("Icons and text"));
		show_icons_and_text->setCheckable(true);
		show_icons_and_text->setChecked(true);
		icon_mode->addAction(show_icons_and_text);
		
		connect(icon_size,SIGNAL(triggered(QAction*)),this,SLOT(iconSizeActionTriggered(QAction*)));
		connect(icon_mode,SIGNAL(triggered(QAction*)),this,SLOT(modeActionTriggered(QAction*)));
		connect(selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),
				 this,SLOT(currentItemChanged(QModelIndex,QModelIndex)));
	}

	ActivityListWidget::~ActivityListWidget() 
	{
	}
	
	void ActivityListWidget::addActivity(Activity* a)
	{
		model->addActivity(a);
	}
	
	void ActivityListWidget::removeActivity(Activity* a)
	{
		model->removeActivity(a);
	}
	
	QSize ActivityListWidget::sizeHint() const
	{
		int max_width = 0;
		for ( int i = 0; i < model->rowCount(); i++) 
		{
			const QModelIndex index = model->index(i,0);
			max_width = qMax(max_width, sizeHintForIndex(index).width());
		}
		
		int view_height = QListView::sizeHint().height();
		
		return QSize(max_width + rect().width() - contentsRect().width(),view_height);
	}

	void ActivityListWidget::showEvent(QShowEvent* event)
	{
		parentWidget()->setMaximumWidth(sizeHint().width());
		parentWidget()->setMinimumWidth(sizeHint().width());
		
		QListView::showEvent(event);
	}
	
	void ActivityListWidget::iconSizeActionTriggered(QAction* act)
	{
		if (act == little_icons)
		{
			icon_size = LITTLE_ICON_SIZE;
			delegate->setIconSize(LITTLE_ICON_SIZE);
			setIconSize(QSize(icon_size,icon_size));
		}
		else if (act == normal_icons)
		{
			icon_size = NORMAL_ICON_SIZE;
			delegate->setIconSize(NORMAL_ICON_SIZE);
			setIconSize(QSize(icon_size,icon_size));
		}
		else
		{
			icon_size = BIG_ICON_SIZE;
			delegate->setIconSize(BIG_ICON_SIZE);
			setIconSize(QSize(icon_size,icon_size));
		}
		
		model->emitLayoutChanged();
		QTimer::singleShot(0,this,SLOT(updateParentSize()));
	}
	
	void ActivityListWidget::modeActionTriggered(QAction* act)
	{
		if (act == show_icons_only)
			mode = ICONS_ONLY;
		else if (act == show_text_only)
			mode = TEXT_ONLY;
		else
			mode = ICONS_AND_TEXT;
		
		model->emitLayoutChanged();
		QTimer::singleShot(0,this,SLOT(updateParentSize()));
	}
	
	void ActivityListWidget::updateParentSize()
	{
		parentWidget()->setMaximumWidth(sizeHint().width());
		parentWidget()->setMinimumWidth(sizeHint().width());
	}
	
	void ActivityListWidget::currentItemChanged(const QModelIndex & sel,const QModelIndex & old)
	{
		Q_UNUSED(old);
		if (sel.isValid() && sel.internalPointer())
			currentActivityChanged((Activity*)sel.internalPointer());
	}
	
	void ActivityListWidget::showConfigMenu(QPoint pos)
	{
		menu->popup(mapToGlobal(pos));
	}
	
	void ActivityListWidget::setCurrentActivity(Activity* act)
	{
		QModelIndex idx = model->indexOf(act);
		if (idx.isValid())
			selectionModel()->select(idx,QItemSelectionModel::Select);
	}
	
	void ActivityListWidget::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("ActivityListWidget");
		mode = (ActivityListDisplayMode)g.readEntry("mode",(int)ICONS_AND_TEXT);
		icon_size = g.readEntry("icon_size",NORMAL_ICON_SIZE);
		
		delegate->setIconSize(icon_size);
		setIconSize(QSize(icon_size,icon_size));
		
		switch (icon_size)
		{
			case LITTLE_ICON_SIZE:
				little_icons->setChecked(true);
				break;
			case NORMAL_ICON_SIZE:
				normal_icons->setChecked(true);
				break;
			case BIG_ICON_SIZE:
				big_icons->setChecked(true);
				break;
		}
		
		switch (mode)
		{
			case ICONS_AND_TEXT:
				show_icons_and_text->setChecked(true);
				break;
			case ICONS_ONLY:
				show_icons_only->setChecked(true);
				break;
			case TEXT_ONLY:
				show_text_only->setChecked(true);
				break;
		}
	}
	
	void ActivityListWidget::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("ActivityListWidget");
		g.writeEntry("mode",(int)mode);
		g.writeEntry("icon_size",icon_size);
	}
}
