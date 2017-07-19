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
#include <KPluginSelector>

#include <util/constants.h>
#include <util/log.h>
#include "pluginmanager.h"
#include "pluginactivity.h"
#include "settings.h"

using namespace bt;

namespace kt
{
    PluginActivity::PluginActivity(PluginManager* pman)
        : Activity(i18n("Plugins"), QStringLiteral("plugins"), 5, 0), pman(pman)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
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

