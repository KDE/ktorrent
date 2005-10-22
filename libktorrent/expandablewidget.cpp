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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qlayout.h>
#include <qsplitter.h>
#include "expandablewidget.h"

namespace kt
{

	ExpandableWidget::ExpandableWidget(QWidget* child,QWidget *parent, const char *name)
			: QWidget(parent, name)
	{
		top_layout = new QHBoxLayout(this);
		child->reparent(this,QPoint(),true);
		// make top of stack
		begin = new StackElement;
		begin->w = child;
		top_layout->add(child);
	}


	ExpandableWidget::~ExpandableWidget()
	{}

	void ExpandableWidget::expand(QWidget* w,Position pos)
	{
		// create new element
		StackElement* se = new StackElement;
		se->w = w;
		se->pos = pos;
		se->next = begin;

		// remove old top from layout
		top_layout->remove(begin->w);
			
		// create new toplevel splitter
		Qt::Orientation orientation = (pos == RIGHT || pos == LEFT) ? Qt::Horizontal : Qt::Vertical;
		QSplitter* s =  new QSplitter(orientation,this);;
		se->s = s;
		
		// reparent w and the bottom widget to s
		w->reparent(s,QPoint(),false);
		if (begin->s)
			begin->s->reparent(s,QPoint(),false);
		else
			begin->w->reparent(s,QPoint(),false);
		
		// add w and the bottom widget to s
		if (pos == RIGHT || pos == ABOVE)
		{
			s->moveToFirst(w);
			s->moveToLast(begin->s ? begin->s : begin->w);
		}
		else
		{
			s->moveToFirst(begin->s ? begin->s : begin->w);
			s->moveToLast(w);
		}
		// make se new top of stack
		begin = se;

		// add toplevel splitter to layout
		top_layout->add(s);
		
		// show s
		s->show();
	}

	void ExpandableWidget::remove(QWidget* w)
	{
		// find the correct stackelement
		StackElement* se = begin;
		StackElement* pse = 0; // element before se
		while (se->w != w && se->next)
		{
			pse = se;
			se = se->next;
		}

		// do not remove last
		if (!se->next)
			return;

		if (!pse)
		{
			// we need to remove the first
			top_layout->remove(se->w);
			// reparent next widget or splitter after se to this
			if (se->next->s)
				se->next->s->reparent(this,QPoint(),false);
			else
				se->next->w->reparent(this,QPoint(),false);
			// reparent current top to 0
			se->w->reparent(0,QPoint(),false);
			// set new top
			begin = se->next;
			// delete splitter and se
			delete se->s;
			delete se;

			if (begin->s)
			{
				begin->s->reparent(this,QPoint(),false);
				top_layout->add(begin->s);
				begin->s->show();
			}
			else
			{
				begin->w->reparent(this,QPoint(),false);
				top_layout->add(begin->w);
				begin->w->show();
			}
		}
		else
		{
			// reparent se->next to pse
			if (se->next->s)
				se->next->s->reparent(pse->s,QPoint(),false);
			else
				se->next->w->reparent(pse->s,QPoint(),false);

			// reparent se to 0
			se->s->reparent(0,QPoint(),false);
			se->w->reparent(0,QPoint(),false);

			// update pse's splitter
			if (pse->pos == RIGHT || pse->pos == ABOVE)
				pse->s->moveToLast(se->next->s ? se->next->s : se->next->w);
			else
				pse->s->moveToFirst(se->next->s ? se->next->s : se->next->w);

			// set pse's next
			pse->next = se->next;
			// delete se and splitter
			delete se->s;
			delete se;
		}
	}
}
#include "expandablewidget.moc"
