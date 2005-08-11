/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Vincent Wagelaar <vincent@ricardis.tudelft.nl>                        *
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
#include <qpixmap.h>
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
	: QFrame(parent, name),curr_tc(0)
{
	setFrameShape(StyledPanel);
	setFrameShadow(Sunken);
	setLineWidth(3);
	setMidLineWidth(3);
	setFixedHeight((int)ceil(fontMetrics().height()*1.5));
	show_excluded = false;
}


ChunkBar::~ChunkBar()
{}

void ChunkBar::setTC(bt::TorrentControl* tc)
{
	curr_tc = tc;
	update();
}

void ChunkBar::drawContents(QPainter *p)
{
	// first draw background
	if (isEnabled())
		p->setBrush(colorGroup().base());
	else
		p->setBrush(colorGroup().background());

	p->setPen(Qt::NoPen);
	p->drawRect(contentsRect());

	p->saveWorldMatrix();

	if (curr_tc)
	{
		Uint32 w = contentsRect().width();
		BitSet bs;
		fillBitSet(bs);
		if (curr_tc->getTotalChunks() > w)
			drawMoreChunksThenPixels(p,bs,colorGroup().highlight());
		else
			drawEqual(p,bs,colorGroup().highlight());

		if (show_excluded && curr_tc->getNumChunksExcluded() > 0)
		{
			BitSet ebs;
			curr_tc->excludedChunksToBitSet(ebs);
			if (curr_tc->getTotalChunks() > w)
				drawMoreChunksThenPixels(p,ebs,Qt::red);
			else
				drawEqual(p,ebs,Qt::red);
		}
	}
	p->restoreWorldMatrix();
}

void ChunkBar::drawEqual(QPainter *p,const BitSet & bs,const QColor & color)
{
	//p->setPen(QPen(colorGroup().highlight(),1,Qt::SolidLine));
	QColor c = color;

	Uint32 w = contentsRect().width();
	double scale = 1.0;
	if (curr_tc->getTotalChunks() != w)
		scale = (double)w / curr_tc->getTotalChunks();
	
	p->setPen(QPen(c,1,Qt::SolidLine));
	p->setBrush(c);
	
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

	QRect r = contentsRect();

	for (QValueList<Range>::iterator i = rs.begin();i != rs.end();++i)
	{
		Range & ra = *i;
		int rw = ra.last - ra.first + 1;
		p->drawRect(r.x() + scale * ra.first,r.y() + 1,rw * scale,r.height() - 1);
	}
}

void ChunkBar::drawMoreChunksThenPixels(QPainter *p,const BitSet & bs,const QColor & color)
{
	Uint32 w = contentsRect().width();

	Uint32 chunks_per_pixel = (int)floor((double)bs.getNumBits() / w);

	QRect r = contentsRect();

	for (Uint32 i = 0;i < w;i++)
	{
		Uint32 num_dl = 0;
		for (Uint32 j = i*chunks_per_pixel;j < (i+1)*chunks_per_pixel;j++)
			if (bs.get(j))
				num_dl++;

		if (num_dl == 0)
			continue;

		double fac = (double)num_dl / chunks_per_pixel;
		QColor c = color.light(255*(1.0-fac));
		p->setPen(QPen(c,1,Qt::SolidLine));
		p->setBrush(c);
		p->drawRect(r.x() + i,r.y()+1,1,r.height() - 1);
	}
}

#include "chunkbar.moc"
