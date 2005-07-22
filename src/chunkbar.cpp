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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qvaluelist.h>
#include <math.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/bitset.h>
#include "chunkbar.h"

using namespace bt;

struct Range
{
	int first,last;
};


ChunkBar::ChunkBar(QWidget *parent, const char *name)
	: QWidget(parent, name),curr_tc(0)
{
	setFixedHeight(30);

	
}


ChunkBar::~ChunkBar()
{}

void ChunkBar::setTC(bt::TorrentControl* tc)
{
	curr_tc = tc;
	update();
}

void ChunkBar::paintEvent(QPaintEvent* )
{
	QPainter p;

	p.begin(this);
	// first draw background
	if (isEnabled())
		p.setBrush(Qt::white);
	else
		p.setBrush(Qt::gray);

	p.setPen(Qt::NoPen);
	p.drawRect(rect());

	p.saveWorldMatrix();
	if (curr_tc)
	{	
		Uint32 w = rect().width();
		// there are 3 possibilities :
		// - for each chunk a pixel
		// - more pixels then chunks
		// - more chunks then pixels
		if (curr_tc->getTotalChunks() < w)
			drawMorePixelsThenChunks(p);
		else if (curr_tc->getTotalChunks() > w)
			drawMoreChunksThenPixels(p);
		else
			drawEqual(p);
	}
	p.restoreWorldMatrix();
	// draw edge last
	p.setBrush(Qt::NoBrush);
	p.setPen(QPen(Qt::black,1,Qt::SolidLine));
	p.drawRect(rect());
	p.end();
}

void ChunkBar::drawEqual(QPainter & p)
{
	BitSet bs;
	curr_tc->toBitSet(bs);

	p.setPen(QPen(Qt::blue,1,Qt::SolidLine));
	p.setBrush(Qt::blue);
	

	QValueList<Range> rs;
	
	for (Uint32 i = 0;i < bs.getNumBits();i++)
	{
		if (!bs.get(i))
			continue;
		
		if (rs.empty())
		{
			Range r = {i,i};
			rs.append(r);
		}
		else
		{
			Range & l = rs.last();
			if (l.last == int(i - 1))
			{
				l.last = i;
			}
			else
			{
				Range r = {i,i};
				rs.append(r);
			}
		}
	}

	QRect r = rect();

	for (QValueList<Range>::iterator i = rs.begin();i != rs.end();i++)
	{
		Range & ra = *i;
		int w = ra.last - ra.first + 1;
		p.drawRect(r.x() + ra.first,r.y()+1,w,r.height() - 1);
	}
}

void ChunkBar::drawMoreChunksThenPixels(QPainter & p)
{
/*	
	p.scale((double)w /(curr_tc->getTotalChunks() - 1),1);
	drawEqual(p);*/
	Uint32 w = rect().width();
	BitSet bs;
	curr_tc->toBitSet(bs);

	Uint32 chunks_per_pixel = (int)ceil((double)bs.getNumBits() / w);

	QRect r = rect();
	
	for (Uint32 i = 0;i < w;i++)
	{
		Uint32 num_dl = 0;
		for (Uint32 j = i*chunks_per_pixel;j < (i+1)*chunks_per_pixel;j++)
			if (bs.get(j))
				num_dl++;

		if (num_dl == 0)
			continue;
		
		QColor c = Qt::blue;
		double fac = (double)num_dl / chunks_per_pixel;
		c.setRgb(255 * (1.0 - fac),255 * (1.0 - fac),c.blue());
		p.setPen(QPen(c,1,Qt::SolidLine));
		p.setBrush(c);
		p.drawRect(r.x() + i,r.y()+1,1,r.height() - 1);
	}
}

void ChunkBar::drawMorePixelsThenChunks(QPainter & p)
{
	Uint32 w = rect().width();
	p.scale((double)w /(curr_tc->getTotalChunks() - 1),1);
	drawEqual(p);
}

#include "chunkbar.moc"
