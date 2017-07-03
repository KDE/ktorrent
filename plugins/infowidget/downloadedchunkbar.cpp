/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#include <QPainter>
#include <QToolTip>

#include "downloadedchunkbar.h"

#include <util/bitset.h>
#include <interfaces/torrentinterface.h>

using namespace bt;

namespace kt
{

    DownloadedChunkBar::DownloadedChunkBar(QWidget* parent) : ChunkBar(parent), curr_tc(nullptr)
    {
    }


    DownloadedChunkBar::~DownloadedChunkBar()
    {}


    const bt::BitSet& DownloadedChunkBar::getBitSet() const
    {
        if (curr_tc)
            return curr_tc->downloadedChunksBitSet();
        else
            return bt::BitSet::null;
    }

    void DownloadedChunkBar::setTC(bt::TorrentInterface* tc)
    {
        curr_tc = tc;
        QSize s = contentsRect().size();
        //Out() << "Pixmap : " << s.width() << " " << s.height() << endl;
        pixmap = QPixmap(s);
        pixmap.fill(palette().color(QPalette::Active, QPalette::Base));
        QPainter painter(&pixmap);
        drawBarContents(&painter);
        update();
    }

    void DownloadedChunkBar::updateBar(bool force)
    {
        const BitSet& bs = getBitSet();
        QSize s = contentsRect().size();
        bool changed = !(curr == bs);

        if (curr_tc)
        {
            BitSet ebs = curr_tc->excludedChunksBitSet();
            ebs.orBitSet(curr_tc->onlySeedChunksBitSet()),
                         changed = changed || !(curr_ebs == ebs);
            curr_ebs = ebs;
        }

        if (changed || pixmap.isNull() || pixmap.width() != s.width() || force)
        {
            pixmap = QPixmap(s);
            pixmap.fill(palette().color(QPalette::Active, QPalette::Base));
            QPainter painter(&pixmap);
            drawBarContents(&painter);
            update();
        }
    }

    void DownloadedChunkBar::drawBarContents(QPainter* p)
    {
        if (!curr_tc)
            return;

        Uint32 w = contentsRect().width();
        const BitSet& bs = getBitSet();
        curr = bs;
        QColor highlight_color = palette().color(QPalette::Active, QPalette::Highlight);
        if (bs.allOn())
            drawAllOn(p, highlight_color, contentsRect());
        else if (curr.getNumBits() > w)
            drawMoreChunksThenPixels(p, bs, highlight_color, contentsRect());
        else
            drawEqual(p, bs, highlight_color, contentsRect());

        const TorrentStats& s = curr_tc->getStats();
        if (s.num_chunks_excluded > 0)
        {
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

