/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KPluginWidget>

#include "pluginactivity.h"
#include "pluginmanager.h"
#include "settings.h"
#include <util/constants.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
PluginActivity::PluginActivity(PluginManager *pman)
    : Activity(i18n("Plugins"), QStringLiteral("plugins"), 5, nullptr)
    , pman(pman)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    pmw = new KPluginWidget(this);
    connect(pmw, &KPluginWidget::changed, this, &PluginActivity::update);
    connect(pmw, &KPluginWidget::pluginConfigSaved, this, &PluginActivity::update);
    layout->addWidget(pmw);
    list = pman->pluginsMetaDataList();
}

PluginActivity::~PluginActivity()
{
}

void PluginActivity::updatePluginList()
{
    pmw->clear();
    pmw->setConfig(KSharedConfig::openConfig()->group("Plugins"));
    pmw->addPlugins(list, i18n("Plugins"));
}

void PluginActivity::update()
{
    pmw->save();
    pman->loadPlugins();
}
}
