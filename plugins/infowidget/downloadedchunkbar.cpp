/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QPainter>
#include <QToolTip>

#include "downloadedchunkbar.h"

#include <interfaces/torrentinterface.h>
#include <util/bitset.h>

using namespace bt;

namespace kt
{
DownloadedChunkBar::DownloadedChunkBar(QWidget *parent)
    : ChunkBar(parent)
    , curr_tc(nullptr)
{
}

DownloadedChunkBar::~DownloadedChunkBar()
{
}

const bt::BitSet &DownloadedChunkBar::getBitSet() const
{
    if (curr_tc)
        return curr_tc->downloadedChunksBitSet();
    else
        return bt::BitSet::null;
}

void DownloadedChunkBar::setTC(bt::TorrentInterface *tc)
{
    curr_tc = tc;
    QSize s = contentsRect().size();
    // Out() << "Pixmap : " << s.width() << " " << s.height() << endl;
    pixmap = QPixmap(s);
    pixmap.fill(palette().color(QPalette::Active, QPalette::Base));
    QPainter painter(&pixmap);
    drawBarContents(&painter);
    update();
}

void DownloadedChunkBar::updateBar(bool force)
{
    const BitSet &bs = getBitSet();
    QSize s = contentsRect().size();
    bool changed = !(curr == bs);

    if (curr_tc) {
        BitSet ebs = curr_tc->excludedChunksBitSet();
        ebs.orBitSet(curr_tc->onlySeedChunksBitSet()), changed = changed || !(curr_ebs == ebs);
        curr_ebs = ebs;
    }

    if (changed || pixmap.isNull() || pixmap.width() != s.width() || force) {
        pixmap = QPixmap(s);
        pixmap.fill(palette().color(QPalette::Active, QPalette::Base));
        QPainter painter(&pixmap);
        drawBarContents(&painter);
        update();
    }
}

void DownloadedChunkBar::drawBarContents(QPainter *p)
{
    if (!curr_tc)
        return;

    Uint32 w = contentsRect().width();
    const BitSet &bs = getBitSet();
    curr = bs;
    QColor highlight_color = palette().color(QPalette::Active, QPalette::Highlight);
    if (bs.allOn())
        drawAllOn(p, highlight_color, contentsRect());
    else if (curr.getNumBits() > w)
        drawMoreChunksThenPixels(p, bs, highlight_color, contentsRect());
    else
        drawEqual(p, bs, highlight_color, contentsRect());

    const TorrentStats &s = curr_tc->getStats();
    if (s.num_chunks_excluded > 0) {
        QColor c = palette().color(QPalette::Active, QPalette::Mid);
        if (curr_ebs.allOn())
            drawAllOn(p, c, contentsRect());
        else if (s.total_chunks > w)
            drawMoreChunksThenPixels(p, curr_ebs, c, contentsRect());
        else
            drawEqual(p, curr_ebs, c, contentsRect());
    }
}
}
