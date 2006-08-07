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
#ifndef CHUNKBAR_H
#define CHUNKBAR_H

#include <qlabel.h>
#include <util/bitset.h>
#include <qpixmap.h>

class QPainter;

namespace kt
{
	class TorrentInterface;
}

namespace bt
{
	class BitSet;
}

namespace kt
{
	
	/**
	* @author Joris Guisson, Vincent Wagelaar
	*
	* Bar which displays BitSets, subclasses need to fill the BitSet.
	* BitSets can represent which chunks are downloaded, which chunks are available
	* and which chunks are excluded.
	*/
	class ChunkBar : public QFrame
	{
		Q_OBJECT
	public:
		ChunkBar(QWidget *parent = 0, const char *name = 0);
		virtual ~ChunkBar();
	
		void setTC(kt::TorrentInterface* tc);
		
		virtual const bt::BitSet & getBitSet() const = 0;
		virtual void drawContents(QPainter *p);
		virtual void updateBar();
	
	private:
		void drawEqual(QPainter *p,const bt::BitSet & bs,const QColor & color);
		void drawMoreChunksThenPixels(QPainter *p,const bt::BitSet & bs,const QColor & color);
		void drawAllOn(QPainter *p,const QColor & color);
		void drawBarContents(QPainter *p);
		
	protected:
		kt::TorrentInterface* curr_tc;
		bool show_excluded;
		bt::BitSet curr,curr_ebs;
		QPixmap pixmap;
	};
}

#endif
