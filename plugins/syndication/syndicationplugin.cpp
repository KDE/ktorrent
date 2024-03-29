/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include "syndicationactivity.h"
#include "syndicationplugin.h"
#include <interfaces/guiinterface.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

K_PLUGIN_CLASS_WITH_JSON(kt::SyndicationPlugin, "ktorrent_syndication.json")

using namespace bt;

namespace kt
{
SyndicationPlugin::SyndicationPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plugin(parent, data, args)
{
    setupActions();
    //  setXMLFile("ktsyndicationpluginui.rc");
    LogSystemManager::instance().registerSystem(i18n("Syndication"), SYS_SYN);
}

SyndicationPlugin::~SyndicationPlugin()
{
    LogSystemManager::instance().unregisterSystem(i18n("Syndication"));
}

void SyndicationPlugin::load()
{
    activity = new SyndicationActivity(this, nullptr);
    connect(add_feed, &QAction::triggered, activity, &SyndicationActivity::addFeed);
    connect(remove_feed, &QAction::triggered, activity, &SyndicationActivity::removeFeed);
    connect(manage_filters, &QAction::triggered, activity, &SyndicationActivity::manageFilters);
    connect(add_filter, &QAction::triggered, activity, &SyndicationActivity::addFilter);
    connect(remove_filter, &QAction::triggered, activity, &SyndicationActivity::removeFilter);
    connect(edit_filter, &QAction::triggered, activity, qOverload<>(&SyndicationActivity::editFilter));
    connect(edit_feed_name, &QAction::triggered, activity, &SyndicationActivity::editFeedName);
    getGUI()->addActivity(activity);
    activity->loadState(KSharedConfig::openConfig());
}

void SyndicationPlugin::unload()
{
    activity->saveState(KSharedConfig::openConfig());
    getGUI()->removeActivity(activity);
    delete activity;
    activity = nullptr;
}

void SyndicationPlugin::setupActions()
{
    KActionCollection *ac = actionCollection();

    add_feed = new QAction(QIcon::fromTheme(QStringLiteral("kt-add-feeds")), i18n("Add Feed"), this);
    ac->addAction(QStringLiteral("add_feed"), add_feed);

    remove_feed = new QAction(QIcon::fromTheme(QStringLiteral("kt-remove-feeds")), i18n("Remove Feed"), this);
    ac->addAction(QStringLiteral("remove_feed"), remove_feed);

    manage_filters = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("Add/Remove Filters"), this);
    ac->addAction(QStringLiteral("manage_filters"), manage_filters);

    edit_feed_name = new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"), this);
    ac->addAction(QStringLiteral("edit_feed_name"), edit_feed_name);

    add_filter = new QAction(QIcon::fromTheme(QStringLiteral("kt-add-filters")), i18n("Add Filter"), this);
    ac->addAction(QStringLiteral("add_filter"), add_filter);

    remove_filter = new QAction(QIcon::fromTheme(QStringLiteral("kt-remove-filters")), i18n("Remove Filter"), this);
    ac->addAction(QStringLiteral("remove_filter"), remove_filter);

    edit_filter = new QAction(QIcon::fromTheme(QStringLiteral("preferences-other")), i18n("Edit Filter"), this);
    ac->addAction(QStringLiteral("edit_filter"), edit_filter);

    remove_filter->setEnabled(false);
    edit_filter->setEnabled(false);
    remove_feed->setEnabled(false);
    manage_filters->setEnabled(false);
}

}

#include "syndicationplugin.moc"
