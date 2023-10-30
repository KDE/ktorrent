/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QNetworkInterface>

#include <KLocalizedString>
#include <Solid/Device>

#include "networkpref.h"
#include "settings.h"
#include <util/log.h>

using namespace bt;

namespace kt
{
NetworkPref::NetworkPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Network"), QStringLiteral("preferences-system-network"), parent)
{
    setupUi(this);
    connect(m_recommended_settings, &QPushButton::clicked, this, &NetworkPref::calculateRecommendedSettings);
    connect(kcfg_utpEnabled, &QCheckBox::toggled, this, &NetworkPref::utpEnabled);
    connect(kcfg_onlyUseUtp, &QCheckBox::toggled, this, &NetworkPref::onlyUseUtpEnabled);
}

NetworkPref::~NetworkPref()
{
}

void NetworkPref::loadSettings()
{
    kcfg_maxDownloadRate->setValue(Settings::maxDownloadRate());
    kcfg_maxUploadRate->setValue(Settings::maxUploadRate());
    kcfg_maxConnections->setValue(Settings::maxConnections());
    kcfg_maxTotalConnections->setValue(Settings::maxTotalConnections());

    combo_networkInterface->clear();
    combo_networkInterface->addItem(QIcon::fromTheme(QStringLiteral("network-wired")), i18n("All interfaces"));

    kcfg_onlyUseUtp->setEnabled(Settings::utpEnabled());
    kcfg_primaryTransportProtocol->setEnabled(Settings::utpEnabled() && !Settings::onlyUseUtp());

    // get all the network devices and add them to the combo box
    const QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : iface_list) {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("network-wired"));
        combo_networkInterface->addItem(icon, iface.humanReadableName());
    }
    const QString iface = Settings::networkInterface();
    int idx = (iface.isEmpty()) ? 0 /*all*/ : combo_networkInterface->findText(iface);
    if (idx < 0) {
        bool ok;
        iface.toInt(&ok);
        if (ok) {
            idx = 0;
        } else {
            combo_networkInterface->addItem(iface);
            idx = combo_networkInterface->findText(iface);
        }
    }
    combo_networkInterface->setCurrentIndex(idx);
}

void NetworkPref::updateSettings()
{
    QString iface = combo_networkInterface->currentText();
    Settings::setNetworkInterface(iface);
}

void NetworkPref::loadDefaults()
{
}

void NetworkPref::utpEnabled(bool on)
{
    kcfg_onlyUseUtp->setEnabled(on);
    kcfg_primaryTransportProtocol->setEnabled(on && !kcfg_onlyUseUtp->isChecked());
}

void NetworkPref::onlyUseUtpEnabled(bool on)
{
    kcfg_primaryTransportProtocol->setEnabled(!on && kcfg_utpEnabled->isChecked());
}

}

#include "moc_networkpref.cpp"
