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

#ifndef KTPLASMACHUNKBAR_H
#define KTPLASMACHUNKBAR_H

#include <QGraphicsWidget>
#include <torrent/chunkbarrenderer.h>
#include <util/bitset.h>

namespace ktplasma
{

    /**
        ChunkBar for the plasma applet
    */
    class ChunkBar : public QGraphicsWidget, public kt::ChunkBarRenderer
    {
    public:
        ChunkBar(QGraphicsItem* parent);
        virtual ~ChunkBar();

        void updateBitSets(int num_chunks, const QByteArray& downloaded, const QByteArray& excluded);
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    private:
        void paintChunks(QPainter* p, const QStyleOptionGraphicsItem* option,
                         const QColor& color, const bt::BitSet& chunks);

    private:
        bt::BitSet downloaded_chunks;
        bt::BitSet excluded_chunks;
    };

}

#endif
