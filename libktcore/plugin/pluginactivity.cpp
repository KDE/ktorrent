/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

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
    : Activity(i18n("Plugins"), QStringLiteral("plugins"), 5, 0)
    , pman(pman)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    pmw = new KPluginWidget(this);
    pmw->setConfig(KSharedConfig::openConfig()->group(QLatin1String("Plugins")));
    connect(pmw, &KPluginWidget::changed, this, &PluginActivity::changed);
    connect(pmw, &KPluginWidget::configCommitted, this, &PluginActivity::changed);
    layout->addWidget(pmw);
}

PluginActivity::~PluginActivity()
{
}

void PluginActivity::updatePluginList()
{
    pmw->addPlugins(KPluginInfo::toMetaData(pman->pluginInfoList()), i18n("Plugins"));
}

void PluginActivity::update()
{
    pmw->save();
    pman->loadPlugins();
}

void PluginActivity::changed()
{
    update();
}
}
