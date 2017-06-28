/***************************************************************************
 *   Copyright (C) 2008 by Petri Damstén <damu@iki.fi>                     *
 *   Copyright (C) 2009 by Amichai Rothman <amichai@amichais.net>          *
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

#ifndef FADINGNAVIGATIONWIDGET_H
#define FADINGNAVIGATIONWIDGET_H

#include <QPushButton>
#include <QGraphicsWidget>
#include <Plasma/PushButton>
#include <Plasma/Frame>
#include "fadingitem.h"

class FadingNavigationWidget : public QObject
{
    Q_OBJECT
public:
    FadingNavigationWidget(QGraphicsWidget* parent);
    virtual ~FadingNavigationWidget();
    void setEnabled(bool enabled);
    Plasma::Frame* frame();
    Plasma::PushButton* nextButton();
    Plasma::PushButton* prevButton();

signals:
    void prevClicked();
    void nextClicked();

protected:
    void initFrame();
    virtual bool eventFilter(QObject* obj, QEvent* event);

private:
    bool navigation_enabled;
    QGraphicsWidget* parent;
    Plasma::Frame* mFrame;
    FadingItem* mFadingItem;
    Plasma::PushButton* mPrevButton;
    Plasma::PushButton* mNextButton;
};

#endif
