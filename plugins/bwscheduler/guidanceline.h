/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGUIDANCELINE_H
#define KTGUIDANCELINE_H

#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

namespace kt
{
/**
    Line displayed when the user is resizing or moving items
    The line has a text item below it to the side
*/
class GuidanceLine : public QGraphicsLineItem
{
public:
    GuidanceLine(qreal x, qreal y, qreal text_offset);
    ~GuidanceLine() override;

    /**
     * Update the guidance line
     * @param nx The nex x start
     * @param ny The new y start
     * @param text The text to display
     */
    void update(qreal nx, qreal ny, const QString &text);

private:
    qreal x;
    qreal y;
    qreal text_offset;
    QGraphicsTextItem *text;
};

}

#endif
