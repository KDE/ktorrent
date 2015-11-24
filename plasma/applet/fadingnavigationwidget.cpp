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

#include "fadingnavigationwidget.h"
#include <QGraphicsLinearLayout>

FadingNavigationWidget::FadingNavigationWidget(QGraphicsWidget* parent) : QObject()
{
    this->parent = parent;
    parent->installEventFilter(this);
    parent->setAcceptHoverEvents(true);
    initFrame();
}

FadingNavigationWidget::~FadingNavigationWidget()
{
}

Plasma::Frame* FadingNavigationWidget::frame()
{
    return mFrame;
}

Plasma::PushButton* FadingNavigationWidget::prevButton()
{
    return mPrevButton;
}

Plasma::PushButton* FadingNavigationWidget::nextButton()
{
    return mNextButton;
}

void FadingNavigationWidget::setEnabled(bool enabled)
{
    navigation_enabled = enabled;
    if (!enabled)
        mFrame->hide();
}

void FadingNavigationWidget::initFrame()
{
    mFrame = new Plasma::Frame(parent);
    mFrame->setZValue(10);
    QGraphicsLinearLayout* l = new QGraphicsLinearLayout();
    mPrevButton = new Plasma::PushButton(mFrame);
    mPrevButton->nativeWidget()->setIcon(QIcon::fromTheme("arrow-left"));
    mPrevButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mPrevButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(mPrevButton, SIGNAL(clicked()), this , SIGNAL(prevClicked()));
    l->addItem(mPrevButton);
    mNextButton = new Plasma::PushButton(mFrame);
    mNextButton->nativeWidget()->setIcon(QIcon::fromTheme("arrow-right"));
    mNextButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mNextButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(mNextButton, SIGNAL(clicked()), this , SIGNAL(nextClicked()));
    l->addItem(mNextButton);
    mFrame->setLayout(l);
    mFrame->setFrameShadow(Plasma::Frame::Raised);
    l->activate(); // makes sure the size is correct
    mFrame->hide();
    mFadingItem = new FadingItem(mFrame);
    mFadingItem->hide();
}

bool FadingNavigationWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == parent && navigation_enabled)
    {
        if (event->type() == QEvent::GraphicsSceneHoverEnter)
        {
            if (!mFadingItem->isVisible())
                mFadingItem->showItem();
        }
        else if (event->type() == QEvent::GraphicsSceneHoverLeave)
        {
            if (mFadingItem->isVisible())
                mFadingItem->hideItem();
        }
    }
    return false;
}

#include "fadingnavigationwidget.moc"
