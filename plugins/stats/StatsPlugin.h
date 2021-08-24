/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef StatsPlugin_H_
#define StatsPlugin_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>

#include <memory>

#include <ConnsTabPage.h>
#include <DisplaySettingsPage.h>
#include <SettingsPage.h>
#include <SpdTabPage.h>
#include <statspluginsettings.h>

namespace kt
{
/** \brief Statistics plugin
\author Krzysztof Kundzicz <athantor@gmail.com>
\version 1.1
*/

class StatsPlugin : public Plugin
{
    Q_OBJECT
public:
    /** \brief Constructor
    \param p Parent
    */
    StatsPlugin(QObject *p, const QVariantList &);
    /// Destructor
    ~StatsPlugin() override;

    void load() override;
    void unload() override;
    void guiUpdate() override;

public Q_SLOTS:
    /// Gather data
    void gatherData();
    /// Settings has been changed
    void settingsChanged();

private:
    /// Speeds tab
    SpdTabPage *pmUiSpd;
    /// Connections tab
    ConnsTabPage *pmUiConns;
    /// Settings Page
    SettingsPage *pmUiSett;
    /// Display settings page
    DisplaySettingsPage *pmDispSett;
    /// Timer
    QTimer pmTmr;

    /// Updates counter
    uint32_t mUpdCtr;
};

} // ns end

#endif
