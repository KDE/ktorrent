/***************************************************************************
 *   Copyright (C) 2008 by Petri Damstén <damu@iki.fi>                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef FADINGITEM_H
#define FADINGITEM_H

#include <QGraphicsItem>
#include <QWeakPointer>

class QPropertyAnimation;

class FadingItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacityValue READ opacityValue WRITE setOpacityValue)
#if QT_VERSION >= 0x040600
    Q_INTERFACES(QGraphicsItem)
#endif
public:
    FadingItem(QGraphicsItem* parent);

    void updatePixmap();
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget = 0);
    void showItem();
    void hideItem();
    bool isVisible() const;
    qreal opacityValue() const;

protected slots:
    void setOpacityValue(qreal opacityValue);
    void animationFinished();

private:
    QPixmap mParent;
    qreal mOpacity;
    QWeakPointer<QPropertyAnimation> mAnimation;
    bool mShowing;
};

#endif
