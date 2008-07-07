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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <Q3MimeSourceFactory>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QList>
#include <QPixmap>
#include <math.h>
#include <qtooltip.h>
#include <klocale.h>
#include <qmime.h>
#include <qimage.h>
#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <util/bitset.h>
#include <torrent/globals.h>
#include "chunkbar.h"

using namespace bt;
using namespace kt;

namespace kt
{
		
	struct Range
	{
		int first,last;
		int fac;
	};
	
	
	static void FillAndFrameBlack(QImage* image, const QColor & color, int size)
	{
		image->fill(color.rgb());
		for (int i = 0; i < size; i++)
		{
			image->setPixel(0, i, 0);
			image->setPixel(size - 1, i, 0);
			image->setPixel(i, 0, 0);
			image->setPixel(i, size - 1, 0);
		}
	}
	
	
	
	static void InitializeToolTipImages(ChunkBar* bar)
	{
		static bool images_initialized = false;
		if (images_initialized)
			return;
		images_initialized = true;
		
		Q3MimeSourceFactory* factory = Q3MimeSourceFactory::defaultFactory();
		
		QImage excluded(16, 16, QImage::Format_RGB32);
		FillAndFrameBlack(&excluded, bar->palette().color(QPalette::Active,QPalette::Mid), 16);
		factory->setImage("excluded_color", excluded);
		
		QImage available(16, 16, QImage::Format_RGB32);
		FillAndFrameBlack(&available, bar->palette().color(QPalette::Active,QPalette::Highlight), 16);
		factory->setImage("available_color", available);
		
		QImage unavailable(16, 16, QImage::Format_RGB32);
		FillAndFrameBlack(&unavailable, bar->palette().color(QPalette::Active,QPalette::Base), 16);
		factory->setImage("unavailable_color", unavailable);
	}
	
	ChunkBar::ChunkBar(QWidget *parent)
		: QFrame(parent)
	{
		setFrameShape(StyledPanel);
		setFrameShadow(Sunken);
		setLineWidth(3);
		setMidLineWidth(3);
		
		InitializeToolTipImages(this);
		setToolTip(i18n("<img src=\"available_color\">&nbsp; - Downloaded Chunks<br>"
				"<img src=\"unavailable_color\">&nbsp; - Chunks to Download<br>"
				"<img src=\"excluded_color\">&nbsp; - Excluded Chunks"));
	}
	
	
	ChunkBar::~ChunkBar()
	{}
	
	void ChunkBar::updateBar()
	{
		const BitSet & bs = getBitSet();
		QSize s = contentsRect().size();
		bool changed = !(curr == bs);

		if (changed || pixmap.isNull() || pixmap.width() != s.width())
		{
			pixmap = QPixmap(s);
			pixmap.fill(palette().color(QPalette::Active,QPalette::Base));
			QPainter painter(&pixmap);
			drawBarContents(&painter);
			update();
		}
	}
	
	void ChunkBar::paintEvent(QPaintEvent* ev)
	{
		QFrame::paintEvent(ev);
		QPainter p(this);
		drawContents(&p);
	}
	
	void ChunkBar::drawContents(QPainter *p)
	{
		// first draw background
		if (isEnabled())
			p->setBrush(palette().color(QPalette::Active,QPalette::Base));
		else
			p->setBrush(palette().color(QPalette::Inactive,QPalette::Base));
	
		p->setPen(Qt::NoPen);
		p->drawRect(contentsRect());
		if (isEnabled())
			p->drawPixmap(contentsRect(),pixmap);
	}
	

	
	void ChunkBar::drawBarContents(QPainter *p)
	{
		Uint32 w = contentsRect().width();
		const BitSet & bs = getBitSet();
		curr = bs;
		QColor highlight_color = palette().color(QPalette::Active,QPalette::Highlight);
		if (bs.allOn())
			drawAllOn(p,highlight_color);
		else if (curr.getNumBits() > w)
			drawMoreChunksThenPixels(p,bs,highlight_color);
		else
			drawEqual(p,bs,highlight_color);
	}
	
	void ChunkBar::drawEqual(QPainter *p,const BitSet & bs,const QColor & color)
	{
		//p->setPen(QPen(colorGroup().highlight(),1,Qt::SolidLine));
		QColor c = color;
	
		Uint32 w = contentsRect().width();
		double scale = 1.0;
		Uint32 total_chunks = bs.getNumBits();
		if (total_chunks != w)
			scale = (double)w / total_chunks;
		
		p->setPen(QPen(c,1,Qt::SolidLine));
		p->setBrush(c);
		
		QList<Range> rs;
		
		for (Uint32 i = 0;i < bs.getNumBits();i++)
		{
			if (!bs.get(i))
				continue;
			
			if (rs.empty())
			{
				Range r = {i,i,0};
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
					Range r = {i,i,0};
					rs.append(r);
				}
			}
		}
	
		QRect r = contentsRect();
	
		for (QList<Range>::iterator i = rs.begin();i != rs.end();++i)
		{
			Range & ra = *i;
			int rw = ra.last - ra.first + 1;
			p->drawRect((int)(scale * ra.first),0,(int)(rw * scale),r.height());
		}
	}
	
	void ChunkBar::drawMoreChunksThenPixels(QPainter *p,const BitSet & bs,const QColor & color)
	{
		Uint32 w = contentsRect().width(); 
		double chunks_per_pixel = (double)bs.getNumBits() / w;
		QList<Range> rs; 

		for (Uint32 i = 0;i < w;i++) 
		{ 
			Uint32 num_dl = 0; 
			Uint32 jStart = (Uint32) (i*chunks_per_pixel); 
			Uint32 jEnd = (Uint32) ((i+1)*chunks_per_pixel+0.5); 
			for (Uint32 j = jStart;j < jEnd;j++) 
				if (bs.get(j)) 
					num_dl++; 
	
			if (num_dl == 0)
				continue;
	
			int fac = int(100*((double)num_dl / (jEnd - jStart)) + 0.5);
			if (rs.empty())
			{
				Range r = {i,i,fac};
				rs.append(r);
			}
			else
			{
				Range & l = rs.last();
				if (l.last == int(i - 1) && l.fac == fac)
				{
					l.last = i;
				}
				else
				{
					Range r = {i,i,fac};
					rs.append(r);
				}
			}
		}
	
		QRect r = contentsRect();
	
		for (QList<Range>::iterator i = rs.begin();i != rs.end();++i)
		{
			Range & ra = *i;
			int rw = ra.last - ra.first + 1;
			int fac = ra.fac;
			QColor c = color;
			if (fac < 100)
			{
				// do some rounding off
				if (fac <= 25)
					fac = 25;
				else if (fac <= 50)
					fac = 45;
				else 
					fac = 65;
				c = color.light(200-fac);
			} 
			p->setPen(QPen(c,1,Qt::SolidLine));
			p->setBrush(c);
			p->drawRect(ra.first,0,rw,r.height());
		}
		
	}
	
	void ChunkBar::drawAllOn(QPainter *p,const QColor & color)
	{
		p->setPen(QPen(color,1,Qt::SolidLine));
		p->setBrush(color);
		QSize s = contentsRect().size();
		p->drawRect(0,0,s.width(),s.height());
	}
}

#include "chunkbar.moc"
