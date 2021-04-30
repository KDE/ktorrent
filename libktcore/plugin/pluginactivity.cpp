/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KPluginSelector>

#include "pluginactivity.h"
#include "pluginmanager.h"
#include "settings.h"
#include <util/constants.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
PluginActivity::PluginActivity(PluginManager *pman)
    : Activity(i18n("Plugins"), QStringLiteral("plugins"), 5, 0)
    , pman(pman)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    pmw = new KPluginSelector(this);
    connect(pmw, &KPluginSelector::changed, this, &PluginActivity::changed);
    connect(pmw, &KPluginSelector::configCommitted, this, &PluginActivity::changed);
    layout->addWidget(pmw);
}

PluginActivity::~PluginActivity()
{
}

void PluginActivity::updatePluginList()
{
    pmw->addPlugins(pman->pluginInfoList(), KPluginSelector::IgnoreConfigFile, i18n("Plugins"));
}

void PluginActivity::update()
{
    pmw->updatePluginsState();
    pman->loadPlugins();
}

void PluginActivity::changed()
{
    update();
}
}
