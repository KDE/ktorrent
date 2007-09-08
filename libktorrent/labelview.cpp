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
#include <QMouseEvent>
#include <kiconloader.h>
#include <kcolorscheme.h>
#include <util/log.h>
#include "labelview.h"

using namespace bt;

namespace kt
{
	LabelViewItem::LabelViewItem(const QString & icon,const QString & title,const QString & description,LabelView* view)
		: odd(false),selected(false)
	{
		setupUi(this);
		icon_lbl->setPixmap(DesktopIcon(icon));
		title_lbl->setText(title);
		description_lbl->setText(description);
		setOdd(false);
		setAutoFillBackground(true);
	}
	
	LabelViewItem::~LabelViewItem()
	{
	}
	
	bool LabelViewItem::operator < (const LabelViewItem & item)
	{
		return title_lbl->text() < item.title_lbl->text();
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
			setForegroundRole(QPalette::HighlightedText);
			setBackgroundRole(QPalette::Highlight);
		}
		else if (odd)
		{
			setForegroundRole(QPalette::WindowText);
			setBackgroundRole(QPalette::Base);
		}
		else
		{
			setForegroundRole(QPalette::WindowText);
			setBackgroundRole(QPalette::AlternateBase);
		}
	}
	
	void LabelViewItem::mousePressEvent(QMouseEvent *e)
	{
		if (e->button() == Qt::LeftButton)
		{
			clicked(this);
		}

		setFocus();
		QWidget::mousePressEvent(e);
	}
	
	class LabelViewBox : public QWidget
	{
		QVBoxLayout* layout;
		int cnt;
	public:
		LabelViewBox(QWidget* parent) : QWidget(parent),cnt(0)
		{
			QPalette p = palette();
			p.setColor(QPalette::Active,QPalette::Window, KColorScheme(QPalette::Active, KColorScheme::View).background().color());
			setPalette(p);
			layout = new QVBoxLayout(this);
			layout->setMargin(0);
			layout->addStretch(0);
		}
		
		virtual ~LabelViewBox()
		{}
		
		void add(LabelViewItem* item)
		{
			item->setParent(this);
			layout->insertWidget(cnt++,item);
			item->show();
		}
		
		void remove(LabelViewItem* item)
		{
			item->hide();
			layout->removeWidget(item);
			item->setParent(0);
			cnt--;
		}
		
		void sorted(QList<LabelViewItem*> & items)
		{
			foreach (LabelViewItem* item,items)
				remove(item);

			foreach (LabelViewItem* item,items)
				add(item);
		}
	};
	

	
	///////////////////////////////////////

	LabelView::LabelView(QWidget *parent)
			: QScrollArea(parent),selected(0)
	{
		setWidgetResizable(true);
		item_box = new LabelViewBox(this);
		setWidget(item_box);
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
	
	void LabelView::updateOddStatus()
	{
		bool odd = true;
		foreach (LabelViewItem* item,items)
		{
			item->setOdd(odd);
			odd = !odd;
		}
	}

	
	void LabelView::removeItem(LabelViewItem* item)
	{
		if (items.contains(item))
		{
			item_box->remove(item);
			items.removeAll(item);
			disconnect(item, SIGNAL(clicked(LabelViewItem*)),
					this, SLOT(onItemClicked(LabelViewItem*)));
			
			// check for selected being equal to item
			if (item == selected)
				selected = 0;
			
			// update odd status of each item
			updateOddStatus();
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
		QList<LabelViewItem*>::iterator i = items.begin();
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
		foreach(LabelViewItem* item,items)
			item->update();
	}

	static bool LabelViewItemCmp(LabelViewItem* a,LabelViewItem* b)
	{
		return *a < *b;
	}
	
	void LabelView::sort()
	{
		qSort(items.begin(),items.end(),LabelViewItemCmp);
		item_box->sorted(items);
		updateOddStatus();
	}
}
#include "labelview.moc"
