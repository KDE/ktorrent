/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTPLUGINACTIVITY_H
#define KTPLUGINACTIVITY_H

#include <QWidget>
#include <interfaces/activity.h>

class KPluginSelector;

namespace kt
{
class PluginManager;

/**
 * @author Joris Guisson
 *
 * Pref page which allows to load and unload plugins.
 */
class PluginActivity : public Activity
{
    Q_OBJECT
public:
    PluginActivity(PluginManager *pman);
    ~PluginActivity() override;

    void updatePluginList();
    void update();
private Q_SLOTS:
    void changed();

private:
    PluginManager *pman;
    KPluginSelector *pmw;
};

}

#endif
