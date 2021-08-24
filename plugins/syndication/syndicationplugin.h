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
    QAction *add_feed = nullptr;
    QAction *remove_feed = nullptr;
    QAction *edit_feed_name = nullptr;
    QAction *add_filter = nullptr;
    QAction *remove_filter = nullptr;
    QAction *edit_filter = nullptr;
    QAction *manage_filters = nullptr;
    SyndicationActivity *activity = nullptr;

    friend class SyndicationActivity;
};

}

#endif
