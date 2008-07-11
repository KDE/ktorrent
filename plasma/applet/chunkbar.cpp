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
#include <QStyleOptionGraphicsItem>
#include <plasma/theme.h>
#include "chunkbar.h"

using namespace bt;

namespace ktplasma
{

	ChunkBar::ChunkBar(QGraphicsItem* parent): QGraphicsWidget(parent),downloaded_chunks(100)
	{
		setAttribute(Qt::WA_NoSystemBackground);
		QFontMetricsF fm(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
		setMaximumHeight(fm.height());
		setMinimumHeight(fm.height());
	}


	ChunkBar::~ChunkBar()
	{
	}
	
	void ChunkBar::updateBitSets(int num_chunks,const QByteArray & downloaded,const QByteArray & excluded)
	{
		bt::BitSet dc((const bt::Uint8*)downloaded.data(),num_chunks);
		bt::BitSet ec((const bt::Uint8*)excluded.data(),num_chunks);
		
		bool modified = false;
		if (!(downloaded_chunks == dc))
		{
			downloaded_chunks = dc;
			modified = true;
		}
		
		if (!(excluded_chunks == ec))
		{
			excluded_chunks = ec;
			modified = true;
		}
		
		if (modified)
			update();
	}
	
	void ChunkBar::paint(QPainter* p,const QStyleOptionGraphicsItem * option, QWidget * widget)
	{
		Q_UNUSED(widget);
		Uint32 w = option->rect.width();
		QColor highlight_color = palette().color(QPalette::Active,QPalette::Highlight);
		if (downloaded_chunks.allOn())
			drawAllOn(p,highlight_color,option->rect);
		else if (downloaded_chunks.getNumBits() > w)
			drawMoreChunksThenPixels(p,downloaded_chunks,highlight_color,option->rect);
		else
			drawEqual(p,downloaded_chunks,highlight_color,option->rect);
		
		if (excluded_chunks.numOnBits() > 0)
		{
			QColor excluded_color = palette().color(QPalette::Active,QPalette::Mid);
			if (excluded_chunks.allOn())
				drawAllOn(p,excluded_color,option->rect);
			else if (excluded_chunks.getNumBits() > w)
				drawMoreChunksThenPixels(p,excluded_chunks,excluded_color,option->rect);
			else
				drawEqual(p,excluded_chunks,excluded_color,option->rect);
		}
	}
}
