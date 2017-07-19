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

#include "fadingitem.h"
#include <QPropertyAnimation>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <KDebug>
#include <Plasma/Animator>

FadingItem::FadingItem(QGraphicsItem* parent)
    : QGraphicsItem(parent),
      mOpacity(0.0),
      mShowing(false)
{
}

void FadingItem::updatePixmap()
{
    QStyleOptionGraphicsItem option;
    mParent = QPixmap(parentItem()->boundingRect().size().toSize() + QSize(1, 1));
    mParent.fill(Qt::transparent);
    QPainter painter(&mParent);
    parentItem()->paint(&painter, &option, 0);
    foreach (QGraphicsItem* item, parentItem()->childItems())
    {
        painter.save();
        painter.translate(item->pos());
        item->paint(&painter, &option, 0);
        painter.restore();
    }
}

QRectF FadingItem::boundingRect() const
{
    return parentItem()->boundingRect();
}

void FadingItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPixmap temp = mParent;
    QPainter tempPainter(&temp);
    QColor color = Qt::black;

    color.setAlphaF(qMin(mOpacity, qreal(0.99)));
    tempPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    tempPainter.fillRect(mParent.rect(), color);
    painter->drawPixmap(QPoint(0, 0), temp);
}

qreal FadingItem::opacityValue() const
{
    return mOpacity;
}

void FadingItem::setOpacityValue(qreal opacityValue)
{
    mOpacity = opacityValue;
    update();
}

void FadingItem::animationFinished()
{
    if (mShowing)
    {
        parentItem()->show();
    }

    hide();
}

void FadingItem::showItem()
{
    QPropertyAnimation* animation = mAnimation.data();
    if (!animation)
    {
        animation = new QPropertyAnimation(this, "opacityValue");
        animation->setDuration(100);
        animation->setEasingCurve(QEasingCurve::InQuad);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        mAnimation = animation;
        connect(animation, &QPropertyAnimation::finished, this, &FadingItem::animationFinished);
    }
    else if (animation->state() == QAbstractAnimation::Running)
    {
        animation->pause();
    }

    mShowing = true;
    updatePixmap();
    show();

    animation->setDirection(QAbstractAnimation::Forward);
    animation->start();

}

void FadingItem::hideItem()
{
    QPropertyAnimation* animation = mAnimation.data();
    if (!animation)
    {
        return;
    }
    else if (animation->state() == QAbstractAnimation::Running)
    {
        animation->pause();
    }

    mShowing = false;
    updatePixmap();
    parentItem()->hide();
    show();

    animation->setDirection(QAbstractAnimation::Backward);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

bool FadingItem::isVisible() const
{
    return mShowing;
}

#include "fadingitem.moc"
