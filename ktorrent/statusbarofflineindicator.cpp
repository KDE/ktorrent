/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-only WITH Qt-Commercial-exception-1.0
*/

#include "statusbarofflineindicator.h"

#include <QLabel>
#include <QVBoxLayout>
#include <kiconloader.h>
#include <klocalizedstring.h>

#include <QIcon>
#include <QNetworkConfigurationManager>

class StatusBarOfflineIndicatorPrivate : public QObject
{
public:
    explicit StatusBarOfflineIndicatorPrivate(StatusBarOfflineIndicator *parent)
        : q(parent)
        , networkConfiguration(new QNetworkConfigurationManager(parent))
    {
    }

    void initialize();
    void _k_networkStatusChanged(bool isOnline);

    StatusBarOfflineIndicator *const q;
    QNetworkConfigurationManager *networkConfiguration;
};

StatusBarOfflineIndicator::StatusBarOfflineIndicator(QWidget *parent)
    : QWidget(parent)
    , d(new StatusBarOfflineIndicatorPrivate(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    QLabel *label = new QLabel(this);
    label->setPixmap(QIcon::fromTheme(QStringLiteral("network-disconnect")).pixmap(KIconLoader::SizeSmall));
    label->setToolTip(i18n("The desktop is offline"));
    layout->addWidget(label);
    d->initialize();
    connect(d->networkConfiguration, &QNetworkConfigurationManager::onlineStateChanged, d, &StatusBarOfflineIndicatorPrivate::_k_networkStatusChanged);
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
