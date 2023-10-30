/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2005 Vincent Wagelaar <vincent@ricardis.tudelft.nl>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QBrush>
#include <QBuffer>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QToolTip>

#include <KLocalizedString>

#include <cmath>

#include "chunkbar.h"
#include "chunkbarrenderer.h"
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <util/bitset.h>
#include <util/log.h>

using namespace bt;
using namespace kt;

namespace kt
{

struct LegendItem {
    QColor color;
    QString label;
};

static QImage FramedColorBlock(const QColor &color, const int size = 16)
{
    QImage image(size, size, QImage::Format_RGB32);
    image.fill(color.rgb());
    for (int i = 0; i < size; i++) {
        image.setPixel(0, i, 0);
        image.setPixel(size - 1, i, 0);
        image.setPixel(i, 0, 0);
        image.setPixel(i, size - 1, 0);
    }
    return image;
}

ChunkBar::ChunkBar(QWidget *parent)
    : QFrame(parent)
    , available_color{palette().color(QPalette::Active, QPalette::Highlight)}
    , unavailable_color{palette().color(QPalette::Active, QPalette::Base)}
    , excluded_color{palette().color(QPalette::Active, QPalette::Mid)}
{
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);
    setLineWidth(3);
    setMidLineWidth(3);

    generateLegend({{available_color, i18n("Downloaded Chunks")}, {unavailable_color, i18n("Chunks to Download")}, {excluded_color, i18n("Excluded Chunks")}});
}

ChunkBar::~ChunkBar()
{
}

void ChunkBar::updateBar(bool force)
{
    const BitSet &bs = getBitSet();
    QSize s = contentsRect().size();

    bool changed = !(curr == bs);

    if (changed || pixmap.isNull() || pixmap.width() != s.width() || force) {
        pixmap = QPixmap(s);
        pixmap.fill(palette().color(QPalette::Active, QPalette::Base));
        QPainter painter(&pixmap);
        drawBarContents(&painter);
        update();
    }
}

void ChunkBar::paintEvent(QPaintEvent *ev)
{
    QFrame::paintEvent(ev);
    QPainter p(this);
    drawContents(&p);
}

void ChunkBar::drawContents(QPainter *p)
{
    // first draw background
    bool enable = isEnabled();
    p->setBrush(palette().color(enable ? QPalette::Active : QPalette::Inactive, QPalette::Base));
    p->setPen(Qt::NoPen); // p->setPen(QPen(Qt::red));
    p->drawRect(contentsRect());
    if (enable)
        p->drawPixmap(contentsRect(), pixmap);
}

void ChunkBar::drawBarContents(QPainter *p)
{
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
}

void ChunkBar::generateLegend(const QList<LegendItem> &legend_items)
{
    QStringList legend;
    for (const auto &legend_item : legend_items) {
        const auto legend_image = FramedColorBlock(legend_item.color);
        QByteArray image_byte_array;
        QBuffer image_buffer(&image_byte_array);
        legend_image.save(&image_buffer, "PNG");
        const auto image_string = QString::fromLatin1(image_byte_array.toBase64().data());
        legend << QStringLiteral("<img align=middle src='data:image/png;base64, %1'> - %2").arg(image_string, legend_item.label);
    }
    setToolTip(legend.join(QStringLiteral("<br>")));
}
}

#include "moc_chunkbar.cpp"
