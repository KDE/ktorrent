/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2005-2007 Vincent Wagelaar <vincent@ricardis.tudelft.nl>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHUNKBAR_H
#define CHUNKBAR_H

#include <QLabel>
#include <QPixmap>

#include "chunkbarrenderer.h"
#include <ktcore_export.h>
#include <util/bitset.h>

class QPainter;

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
class KTCORE_EXPORT ChunkBar : public QFrame, public ChunkBarRenderer
{
    Q_OBJECT
public:
    ChunkBar(QWidget *parent);
    ~ChunkBar() override;

    virtual const bt::BitSet &getBitSet() const = 0;
    void drawContents(QPainter *p);
    virtual void updateBar(bool force = false);

protected:
    virtual void drawBarContents(QPainter *p);
    void paintEvent(QPaintEvent *ev) override;

protected:
    bt::BitSet curr;
    QPixmap pixmap;
};
}

#endif
