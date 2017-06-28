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

    ChunkBar::ChunkBar(QGraphicsItem* parent): QGraphicsWidget(parent), downloaded_chunks(100)
    {
        setAttribute(Qt::WA_NoSystemBackground);
        QFontMetricsF fm(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        setMaximumHeight(fm.height());
        setMinimumHeight(fm.height());
    }

    ChunkBar::~ChunkBar()
    {
    }

    void ChunkBar::updateBitSets(int num_chunks, const QByteArray& downloaded, const QByteArray& excluded)
    {
        bt::BitSet dc((const bt::Uint8*)downloaded.data(), num_chunks);
        bt::BitSet ec((const bt::Uint8*)excluded.data(), num_chunks);

        if (downloaded_chunks != dc || excluded_chunks != ec)
        {
            downloaded_chunks = dc;
            excluded_chunks = ec;
            update();
        }
    }

    void ChunkBar::paintChunks(QPainter* p, const QStyleOptionGraphicsItem* option,
                               const QColor& color, const bt::BitSet& chunks)
    {
        Uint32 w = option->rect.width();
        if (chunks.allOn())
            drawAllOn(p, color, option->rect);
        else if (chunks.getNumBits() > w)
            drawMoreChunksThenPixels(p, chunks, color, option->rect);
        else
            drawEqual(p, chunks, color, option->rect);
    }

    void ChunkBar::paint(QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);
        QColor highlight_color = palette().color(QPalette::Active, QPalette::Highlight);
        paintChunks(p, option, highlight_color, downloaded_chunks);
        if (excluded_chunks.numOnBits() > 0)
        {
            QColor excluded_color = palette().color(QPalette::Active, QPalette::Mid);
            paintChunks(p, option, excluded_color, excluded_chunks);
        }
    }
}
