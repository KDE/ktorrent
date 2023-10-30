/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "availabilitychunkbar.h"

#include <QPainter>
#include <QToolTip>

#include <KLocalizedString>

#include <interfaces/torrentinterface.h>
#include <util/bitset.h>

namespace kt
{
AvailabilityChunkBar::AvailabilityChunkBar(QWidget *parent)
    : ChunkBar(parent)
    , curr_tc(nullptr)
{
    generateLegend({{available_color, i18n("Available Chunks")}, {unavailable_color, i18n("Unavailable Chunks")}, {excluded_color, i18n("Excluded Chunks")}});
}

AvailabilityChunkBar::~AvailabilityChunkBar()
{
}

const bt::BitSet &AvailabilityChunkBar::getBitSet() const
{
    if (curr_tc)
        return curr_tc->availableChunksBitSet();
    else
        return bt::BitSet::null;
}

void AvailabilityChunkBar::setTC(bt::TorrentInterface *tc)
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
}

#include "moc_availabilitychunkbar.cpp"
