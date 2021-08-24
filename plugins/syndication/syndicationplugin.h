/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSYNDICATIONPLUGIN_H
#define KTSYNDICATIONPLUGIN_H

#include <interfaces/plugin.h>

class QAction;

namespace kt
{
class SyndicationActivity;

/**
    @author
*/
class SyndicationPlugin : public Plugin
{
public:
    SyndicationPlugin(QObject *parent, const QVariantList &args);
    ~SyndicationPlugin();

    void load() override;
    void unload() override;

private:
    void setupActions();

private:
    QAction *add_feed;
    QAction *remove_feed;
    QAction *edit_feed_name;
    QAction *add_filter;
    QAction *remove_filter;
    QAction *edit_filter;
    QAction *manage_filters;
    SyndicationActivity *activity;

    friend class SyndicationActivity;
};

}

#endif
