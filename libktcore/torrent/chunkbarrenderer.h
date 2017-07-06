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

#ifndef KTCHUNKBARRENDERER_H
#define KTCHUNKBARRENDERER_H

#include <ktcore_export.h>

namespace bt
{
    class BitSet;
}

namespace kt
{

    /**
        Class which renders a chunkbar to a a QPainter
    */
    class KTCORE_EXPORT ChunkBarRenderer
    {
    public:
        ChunkBarRenderer();
        ~ChunkBarRenderer();

        void drawEqual(QPainter* p, const bt::BitSet& bs, const QColor& color, const QRect& contents_rect);
        void drawMoreChunksThenPixels(QPainter* p, const bt::BitSet& bs, const QColor& color, const QRect& contents_rect);
        void drawAllOn(QPainter* p, const QColor& color, const QRect& contents_rect);
    };

}

#endif
