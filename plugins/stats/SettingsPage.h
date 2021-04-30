/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SettingsPage_H_
#define SettingsPage_H_

#include <QWidget>

#include <KColorButton>

#include <interfaces/prefpageinterface.h>

#include <libktcore/settings.h>

#include <PluginPage.h>
#include <statspluginsettings.h>

#include <ui_Settings.h>

namespace kt
{
/** \brief Settings page
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class SettingsPage : public PrefPageInterface, public Ui_StatsSettingsWgt
{
    Q_OBJECT

public:
    /** \brief Constructor
    \param  p Parent
    */
    SettingsPage(QWidget *p);
    /// Destructor
    ~SettingsPage() override;

public Q_SLOTS:
    void updateSettings() override;

private Q_SLOTS:
    void UpdGuiUpdatesToMs(int);

Q_SIGNALS:
    /// Settings has been applied
    void Applied();
};

} // ns end

#endif
