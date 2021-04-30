/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    void drawEqual(QPainter *p, const bt::BitSet &bs, const QColor &color, const QRect &contents_rect);
    void drawMoreChunksThenPixels(QPainter *p, const bt::BitSet &bs, const QColor &color, const QRect &contents_rect);
    void drawAllOn(QPainter *p, const QColor &color, const QRect &contents_rect);
};

}

#endif
