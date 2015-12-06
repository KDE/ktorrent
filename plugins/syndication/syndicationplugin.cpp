/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kgenericfactory.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include <interfaces/guiinterface.h>
#include "syndicationplugin.h"
#include "syndicationactivity.h"

K_EXPORT_COMPONENT_FACTORY(ktsyndicationplugin, KGenericFactory<kt::SyndicationPlugin>("ktsyndicationplugin"))

using namespace bt;

namespace kt
{

    SyndicationPlugin::SyndicationPlugin(QObject* parent, const QStringList& args): Plugin(parent), add_feed(0)
    {
        Q_UNUSED(args);
        setupActions();
        //  setXMLFile("ktsyndicationpluginui.rc");
        LogSystemManager::instance().registerSystem(i18n("Syndication"), SYS_SYN);
    }

    SyndicationPlugin::~SyndicationPlugin()
    {
        LogSystemManager::instance().unregisterSystem(i18n("Syndication"));
    }

    bool SyndicationPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }

    void SyndicationPlugin::load()
    {
        activity = new SyndicationActivity(this, 0);
        connect(add_feed, SIGNAL(triggered()), activity, SLOT(addFeed()));
        connect(remove_feed, SIGNAL(triggered()), activity, SLOT(removeFeed()));
        connect(manage_filters, SIGNAL(triggered()), activity, SLOT(manageFilters()));
        connect(add_filter, SIGNAL(triggered()), activity, SLOT(addFilter()));
        connect(remove_filter, SIGNAL(triggered()), activity, SLOT(removeFilter()));
        connect(edit_filter, SIGNAL(triggered()), activity, SLOT(editFilter()));
        connect(edit_feed_name, SIGNAL(triggered()), activity, SLOT(editFeedName()));
        getGUI()->addActivity(activity);
        activity->loadState(KSharedConfig::openConfig());
    }

    void SyndicationPlugin::unload()
    {
        activity->saveState(KSharedConfig::openConfig());
        getGUI()->removeActivity(activity);
        delete activity;
        activity = 0;
    }





    void SyndicationPlugin::setupActions()
    {
        KActionCollection* ac = actionCollection();

        add_feed = new QAction(QIcon::fromTheme("kt-add-feeds"), i18n("Add Feed"), this);
        ac->addAction("add_feed", add_feed);

        remove_feed = new QAction(QIcon::fromTheme("kt-remove-feeds"), i18n("Remove Feed"), this);
        ac->addAction("remove_feed", remove_feed);

        manage_filters = new QAction(QIcon::fromTheme("view-filter"), i18n("Add/Remove Filters"), this);
        ac->addAction("manage_filters", manage_filters);

        edit_feed_name = new QAction(QIcon::fromTheme("edit-rename"), i18n("Rename"), this);
        ac->addAction("edit_feed_name", edit_feed_name);

        add_filter = new QAction(QIcon::fromTheme("kt-add-filters"), i18n("Add Filter"), this);
        ac->addAction("add_filter", add_filter);

        remove_filter = new QAction(QIcon::fromTheme("kt-remove-filters"), i18n("Remove Filter"), this);
        ac->addAction("remove_filter", remove_filter);

        edit_filter = new QAction(QIcon::fromTheme("preferences-other"), i18n("Edit Filter"), this);
        ac->addAction("edit_filter", edit_filter);

        remove_filter->setEnabled(false);
        edit_filter->setEnabled(false);
        remove_feed->setEnabled(false);
        manage_filters->setEnabled(false);
    }


}
