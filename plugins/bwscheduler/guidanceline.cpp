/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "guidanceline.h"

#include "bwschedulerpluginsettings.h"
#include <QFontMetricsF>
#include <QPen>

namespace kt
{
GuidanceLine::GuidanceLine(qreal x, qreal y, qreal text_offset)
    : QGraphicsLineItem()
    , x(x)
    , y(y)
    , text_offset(text_offset)
{
    QPen pen(SchedulerPluginSettings::scheduleLineColor());
    pen.setStyle(Qt::DashLine);
    setPen(pen);
    setZValue(5);

    const QString ZERO = QStringLiteral("00:00");
    text = new QGraphicsTextItem(ZERO, this);
    text->setPos(text_offset, y);

    QFontMetricsF fm(text->font());
    qreal xe = text_offset + fm.width(ZERO);
    setLine(x, y, xe, y);
}

GuidanceLine::~GuidanceLine()
{
}

void GuidanceLine::update(qreal nx, qreal ny, const QString &txt)
{
    x = nx;
    y = ny;
    text->setPlainText(txt);
    text->setPos(text_offset, y);
    QFontMetricsF fm(text->font());
    qreal xe = text_offset + fm.width(txt);
    setLine(x, y, xe, y);
}

}
