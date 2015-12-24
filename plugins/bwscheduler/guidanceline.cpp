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
#include "guidanceline.h"

#include <QPen>
#include <QFontMetricsF>
#include "bwschedulerpluginsettings.h"

namespace kt
{

    GuidanceLine::GuidanceLine(qreal x, qreal y, qreal text_offset)
        : QGraphicsLineItem(), x(x), y(y), text_offset(text_offset)
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

    void GuidanceLine::update(qreal nx, qreal ny, const QString& txt)
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
