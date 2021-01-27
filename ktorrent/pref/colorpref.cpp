/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>

#include "colorpref.h"
#include "settings.h"

namespace kt
{
ColorPref::ColorPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Colors"), QStringLiteral("preferences-desktop-color"), parent)
{
    setupUi(this);

    connect(btnResetColors, &QPushButton::clicked, [=]() {
        // set default values for current pref page only
        kcfg_okTorrentColor->setColor(QColor(40, 205, 40));
        kcfg_stalledTorrentColor->setColor(QColor(255, 174, 0));
        kcfg_errorTorrentColor->setColor(QColor(Qt::red));

        kcfg_highlightTorrentNameByTrackerStatus->setChecked(true);
        kcfg_okTrackerConnectionColor->setColor(QColor(40, 205, 40));
        kcfg_warningsTrackerConnectionColor->setColor(QColor(255, 80, 0));
        kcfg_timeoutTrackerConnectionColor->setColor(QColor(0, 170, 110));
        kcfg_noTrackerConnectionColor->setColor(QColor(Qt::red));

        kcfg_goodShareRatioColor->setColor(QColor(40, 205, 40));
        kcfg_lowShareRatioColor->setColor(QColor(Qt::red));
    });
}

ColorPref::~ColorPref()
{
}

void ColorPref::loadSettings()
{
    kcfg_okTorrentColor->setColor(Settings::okTorrentColor());
    kcfg_stalledTorrentColor->setColor(Settings::stalledTorrentColor());
    kcfg_errorTorrentColor->setColor(Settings::errorTorrentColor());

    kcfg_highlightTorrentNameByTrackerStatus->setChecked(Settings::highlightTorrentNameByTrackerStatus());
    kcfg_okTrackerConnectionColor->setColor(Settings::okTrackerConnectionColor());
    kcfg_warningsTrackerConnectionColor->setColor(Settings::warningsTrackerConnectionColor());
    kcfg_timeoutTrackerConnectionColor->setColor(Settings::timeoutTrackerConnectionColor());
    kcfg_noTrackerConnectionColor->setColor(Settings::noTrackerConnectionColor());

    kcfg_goodShareRatioColor->setColor(Settings::goodShareRatioColor());
    kcfg_lowShareRatioColor->setColor(Settings::lowShareRatioColor());
}

void ColorPref::loadDefaults()
{
    loadSettings();
}

}
