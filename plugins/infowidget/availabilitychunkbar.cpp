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
 *   51 Fran`in Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "availabilitychunkbar.h"

#include <QPainter>
#include <QToolTip>

#include <KLocalizedString>

#include <util/bitset.h>
#include <interfaces/torrentinterface.h>

namespace kt
{

    AvailabilityChunkBar::AvailabilityChunkBar(QWidget* parent) : ChunkBar(parent), curr_tc(nullptr)
    {
        setToolTip(i18n("<img src=\"available_color\">&nbsp; - Available Chunks<br>"
                        "<img src=\"unavailable_color\">&nbsp; - Unavailable Chunks<br>"
                        "<img src=\"excluded_color\">&nbsp; - Excluded Chunks"));
    }


    AvailabilityChunkBar::~AvailabilityChunkBar()
    {
    }


    const bt::BitSet& AvailabilityChunkBar::getBitSet() const
    {
        if (curr_tc)
            return curr_tc->availableChunksBitSet();
        else
            return bt::BitSet::null;
    }

    void AvailabilityChunkBar::setTC(bt::TorrentInterface* tc)
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
}

