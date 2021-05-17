/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QPainter>

#include "chunkbarrenderer.h"
#include <util/bitset.h>

using namespace bt;

namespace kt
{
struct Range {
    Uint32 first, last;
    int fac;
};

ChunkBarRenderer::ChunkBarRenderer()
{
}

ChunkBarRenderer::~ChunkBarRenderer()
{
}

void ChunkBarRenderer::drawEqual(QPainter *p, const BitSet &bs, const QColor &color, const QRect &contents_rect)
{
    // p->setPen(QPen(colorGroup().highlight(),1,Qt::SolidLine));
    QColor c = color;

    Uint32 w = contents_rect.width();
    double scale = 1.0;
    Uint32 total_chunks = bs.getNumBits();
    if (total_chunks != w)
        scale = (double)w / total_chunks;

    p->setPen(QPen(c, 1, Qt::SolidLine));
    p->setBrush(c);

    QVector<Range> rs;

    for (Uint32 i = 0; i < bs.getNumBits(); i++) {
        if (!bs.get(i))
            continue;

        if (rs.empty()) {
            Range r = {i, i, 0};
            rs.append(r);
        } else {
            Range &l = rs.last();
            if (l.last == i - 1) {
                l.last = i;
            } else {
                Range r = {i, i, 0};
                rs.append(r);
            }
        }
    }

    QRect r = contents_rect;

    for (auto i = rs.constBegin(); i != rs.constEnd(); ++i) {
        const Range &ra = *i;
        int rw = ra.last - ra.first + 1;
        p->drawRect((int)(scale * ra.first), 0, (int)(rw * scale), r.height());
    }
}

void ChunkBarRenderer::drawMoreChunksThenPixels(QPainter *p, const BitSet &bs, const QColor &color, const QRect &contents_rect)
{
    Uint32 w = contents_rect.width();
    double chunks_per_pixel = (double)bs.getNumBits() / w;
    QVector<Range> rs;

    for (Uint32 i = 0; i < w; i++) {
        Uint32 num_dl = 0;
        Uint32 jStart = (Uint32)(i * chunks_per_pixel);
        Uint32 jEnd = (Uint32)((i + 1) * chunks_per_pixel + 0.5);
        for (Uint32 j = jStart; j < jEnd; j++)
            if (bs.get(j))
                num_dl++;

        if (num_dl == 0)
            continue;

        int fac = int(100 * ((double)num_dl / (jEnd - jStart)) + 0.5);
        if (rs.empty()) {
            Range r = {i, i, fac};
            rs.append(r);
        } else {
            Range &l = rs.last();
            if (l.last == i - 1 && l.fac == fac) {
                l.last = i;
            } else {
                Range r = {i, i, fac};
                rs.append(r);
            }
        }
    }

    QRect r = contents_rect;

    for (auto i = rs.constBegin(); i != rs.constEnd(); ++i) {
        const Range &ra = *i;
        int rw = ra.last - ra.first + 1;
        int fac = ra.fac;
        QColor c = color;
        if (fac < 100) {
            // do some rounding off
            if (fac <= 25)
                fac = 25;
            else if (fac <= 50)
                fac = 45;
            else
                fac = 65;
            c = color.lighter(200 - fac);
        }
        p->setPen(QPen(c, 1, Qt::SolidLine));
        p->setBrush(c);
        p->drawRect(ra.first, 0, rw, r.height());
    }
}

void ChunkBarRenderer::drawAllOn(QPainter *p, const QColor &color, const QRect &contents_rect)
{
    p->setPen(QPen(color, 1, Qt::SolidLine));
    p->setBrush(color);
    QSize s = contents_rect.size();
    p->drawRect(0, 0, s.width(), s.height());
}
}
