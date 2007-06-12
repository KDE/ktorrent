/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#include <qlayout.h>
#include <qlabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <util/log.h>
#include "labelview.h"

using namespace bt;

namespace kt
{
	LabelViewItem::LabelViewItem(const QString & icon,const QString & title,const QString & description,LabelView* view)
		: LabelViewItemBase(view),odd(false),selected(false)
	{
		icon_lbl->setPixmap(DesktopIcon(icon));
		title_lbl->setText(title);
		description_lbl->setText(description);
		setOdd(false);
	}
	
	LabelViewItem::~LabelViewItem()
	{
	}
	
	void LabelViewItem::setTitle(const QString & title)
	{
		title_lbl->setText(title);
	}
	
	void LabelViewItem::setDescription(const QString & d)
	{
		description_lbl->setText(d);
	}
	
	void LabelViewItem::setIcon(const QString & icon)
	{
		icon_lbl->setPixmap(DesktopIcon(icon));
	}
	
	void LabelViewItem::setOdd(bool o)
	{
		odd = o;
		setSelected(selected);
	}
	
	void LabelViewItem::setSelected(bool sel)
	{
		selected = sel;

		if (selected)
		{
			setPaletteBackgroundColor(KGlobalSettings::highlightColor());
			setPaletteForegroundColor(KGlobalSettings::highlightedTextColor());
		}
		else if (odd)
		{
			setPaletteBackgroundColor(KGlobalSettings::baseColor());
			setPaletteForegroundColor(KGlobalSettings::textColor());
		}
		else
		{
			setPaletteBackgroundColor(KGlobalSettings::alternateBackgroundColor());
			setPaletteForegroundColor(KGlobalSettings::textColor());
		}
	}
	
	void LabelViewItem::mousePressEvent(QMouseEvent *e)
	{
		if (e->button() == QMouseEvent::LeftButton)
		{
			clicked(this);
		}

		setFocus();
		QWidget::mousePressEvent(e);
	}
	
	class LabelViewBox : public QWidget
	{
		QVBoxLayout* layout;
	public:
		LabelViewBox(QWidget* parent) : QWidget(parent)
		{
			setPaletteBackgroundColor(KGlobalSettings::baseColor());
			layout = new QVBoxLayout(this);
			layout->setMargin(0);
		}
		
		virtual ~LabelViewBox()
		{}
		
		void add(LabelViewItem* item)
		{
			item->reparent(this,QPoint(0,0));
			layout->add(item);
			item->show();
		}
		
		void remove(LabelViewItem* item)
		{
			item->hide();
			layout->remove(item);
			item->reparent(0,QPoint(0,0));
		}
		
		
	};
	

	
	///////////////////////////////////////

	LabelView::LabelView ( QWidget *parent, const char *name )
			: QScrollView ( parent, name ),selected(0)
	{
		item_box = new LabelViewBox(this->viewport());
		setResizePolicy(QScrollView::AutoOneFit);
	
		addChild(item_box, 0, 0);
		item_box->show();
	}


	LabelView::~LabelView()
	{}

	void LabelView::addItem(LabelViewItem* item)
	{
		item_box->add(item);
		items.append(item);
		item->setOdd(items.count() % 2 == 1);
		
		connect(item, SIGNAL(clicked(LabelViewItem*)),
				this, SLOT(onItemClicked(LabelViewItem*)));
	}
	
	void LabelView::removeItem(LabelViewItem* item)
	{
		if (items.contains(item))
		{
			item_box->remove(item);
			items.remove(item);
			disconnect(item, SIGNAL(clicked(LabelViewItem*)),
					this, SLOT(onItemClicked(LabelViewItem*)));
			
			// check for selected being equal to item
			if (item == selected)
				selected = 0;
			
			// update odd status of each item
			bool odd = false;
			QValueList<LabelViewItem*>::iterator i = items.begin();
			while (i != items.end())
			{
				LabelViewItem* item = *i;
				item->setOdd(odd);
				odd = !odd;
				i++;
			}
		}
	}
	
	void LabelView::onItemClicked(LabelViewItem* it)
	{
		if (selected == it)
			return;
		
		if (selected)
			selected->setSelected(false);
		
		selected = it;
		selected->setSelected(true);
		currentChanged(selected);
	}
	
	void LabelView::clear()
	{
		QValueList<LabelViewItem*>::iterator i = items.begin();
		while (i != items.end())
		{
			LabelViewItem* item = *i;
			item_box->remove(item);
			i = items.erase(i);
			delete item;
		}
		selected = 0;
	}
	
	void LabelView::update()
	{
		QValueList<LabelViewItem*>::iterator i = items.begin();
		while (i != items.end())
		{
			LabelViewItem* item = *i;
			item->update();
			i++;
		}
	}

}
#include "labelview.moc"
