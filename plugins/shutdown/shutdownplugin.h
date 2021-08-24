/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_SHUTDOWNPLUGIN_H
#define KT_SHUTDOWNPLUGIN_H

#include <interfaces/plugin.h>

class KToggleAction;

namespace kt
{
class ShutdownRuleSet;

class ShutdownPlugin : public kt::Plugin
{
    Q_OBJECT
public:
    ShutdownPlugin(QObject *parent, const QVariantList &args);
    ~ShutdownPlugin() override;

    void unload() override;
    void load() override;

public Q_SLOTS:
    void shutdownComputer();
    void lock();
    void suspendToDisk();
    void suspendToRam();

private Q_SLOTS:
    void shutdownToggled(bool on);
    void configureShutdown();
    void updateAction();

private:
    KToggleAction *shutdown_enabled;
    QAction *configure_shutdown;
    ShutdownRuleSet *rules;
};

}

#endif // KT_SHUTDOWNPLUGIN_H
