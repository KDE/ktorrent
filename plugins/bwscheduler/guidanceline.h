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
        ~GuidanceLine();

        /**
         * Update the guidance line
         * @param nx The nex x start
         * @param ny The new y start
         * @param text The text to display
         */
        void update(qreal nx, qreal ny, const QString& text);
    private:
        qreal x;
        qreal y;
        qreal text_offset;
        QGraphicsTextItem* text;
    };

}

#endif
