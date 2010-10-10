/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#include "videochunkbar.h"
#include <KApplication>
#include <QStyle>
#include <QPainter>
#include <QStyleOption>
#include <KIcon>


namespace kt
{
	VideoChunkBar::VideoChunkBar(const kt::MediaFileRef& mf, QWidget* parent)
		: ChunkBar(parent),mfile(mf),current_chunk(0)
	{
		MediaFile::Ptr file = mfile.mediaFile();
		if (file)
		{
			bt::TorrentFileStream::Ptr stream = file->stream().toStrongRef();
			if (stream)
				connect(stream.data(),SIGNAL(readyRead()),this,SLOT(updateChunkBar()));
			
			updateBitSet();
		}
	}
	
	VideoChunkBar::~VideoChunkBar()
	{
	}
	
	void VideoChunkBar::updateBitSet()
	{
		MediaFile::Ptr file = mfile.mediaFile();
		if (file)
		{
			bt::TorrentFileStream::Ptr stream = file->stream().toStrongRef();
			if (stream)
				bitset = stream->chunksBitSet();
			else
				bitset.clear();
		}
		else
			bitset.clear();
	}

	void VideoChunkBar::updateChunkBar()
	{
		updateBitSet();
		updateBar(true);
	}
	
	void VideoChunkBar::timeElapsed(qint64 time)
	{
		Q_UNUSED(time);
		MediaFile::Ptr file = mfile.mediaFile();
		if (!file)
			return;
		
		bt::TorrentFileStream::Ptr stream = file->stream().toStrongRef();
		if (!stream)
			return;
		
		if (current_chunk != stream->currentChunk())
			updateChunkBar();
	}
	
	void VideoChunkBar::drawBarContents(QPainter* p)
	{
		ChunkBar::drawBarContents(p);
		
		MediaFile::Ptr file = mfile.mediaFile();
		if (!file)
			return;
		
		bt::TorrentFileStream::Ptr stream = file->stream().toStrongRef();
		if (!stream)
			return;
		
		if (position_pixmap.isNull())
		{
			KIcon icon = KIcon("arrow-up");
			position_pixmap = icon.pixmap(contentsRect().height(),QIcon::Normal,QIcon::On);
		}
		
		current_chunk = stream->currentChunk();
		qreal f = (qreal)current_chunk / bitset.getNumBits();
		int x = (int)(f * contentsRect().width()); 
		
		/*
		QColor color = palette().color(QPalette::Active,QPalette::Foreground);
		p->setPen(QPen(color,1,Qt::SolidLine));
		p->drawLine(x,0,x,contentsRect().height());
		*/
		/*
		QRect r = position_pixmap.rect();
		QRect dr(x - r.width() / 2,contentsRect().height() - r.height() ,r.width(),r.height());
		p->drawPixmap(dr,position_pixmap,r);
		*/
		
		QStyleOptionSlider option;
		option.orientation = Qt::Horizontal;
		option.minimum = 0;
		option.maximum = bitset.getNumBits();
		option.tickPosition = QSlider::NoTicks;
		//option.sliderValue = current_chunk;
		option.sliderPosition = current_chunk;
		option.rect = QRect(x - 5,0,11,contentsRect().height());
		
		KApplication::style()->drawControl(QStyle::CE_ScrollBarSlider,&option,p,this);
	}


	const bt::BitSet& VideoChunkBar::getBitSet() const
	{
		return bitset;
	}
}
