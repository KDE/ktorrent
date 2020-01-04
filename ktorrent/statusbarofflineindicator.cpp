/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "statusbarofflineindicator.h"

#include <QLabel>
#include <QVBoxLayout>
#include <kiconloader.h>
#include <klocalizedstring.h>

#include <QIcon>
#include <QNetworkConfigurationManager>

class StatusBarOfflineIndicatorPrivate
{
public:
    explicit StatusBarOfflineIndicatorPrivate(StatusBarOfflineIndicator *parent)
        : q(parent)
        , networkConfiguration(new QNetworkConfigurationManager(parent))
    {
    }

    void initialize();
    void _k_networkStatusChanged(bool isOnline);

    StatusBarOfflineIndicator * const q;
    QNetworkConfigurationManager *networkConfiguration;
};

StatusBarOfflineIndicator::StatusBarOfflineIndicator(QWidget *parent)
    : QWidget(parent),
      d(new StatusBarOfflineIndicatorPrivate(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    QLabel *label = new QLabel(this);
    label->setPixmap(QIcon::fromTheme(QStringLiteral("network-disconnect")).pixmap(KIconLoader::SizeSmall));
    label->setToolTip(i18n("The desktop is offline"));
    layout->addWidget(label);
    d->initialize();
    connect(d->networkConfiguration, SIGNAL(onlineStateChanged(bool)),
            SLOT(_k_networkStatusChanged(bool)));
}

StatusBarOfflineIndicator::~StatusBarOfflineIndicator()
{
    delete d;
}

void StatusBarOfflineIndicatorPrivate::initialize()
{
    _k_networkStatusChanged(networkConfiguration->isOnline());
}

void StatusBarOfflineIndicatorPrivate::_k_networkStatusChanged(bool isOnline)
{
    if (isOnline) {
        q->hide();
    } else {
        q->show();
    }
}

#include "moc_statusbarofflineindicator.cpp"
