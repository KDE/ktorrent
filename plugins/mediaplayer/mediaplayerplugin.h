/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTMEDIAPLAYERPLUGIN_H
#define KTMEDIAPLAYERPLUGIN_H

#include <QModelIndex>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>

namespace kt
{
class MediaPlayerActivity;

/**
    @author
*/
class MediaPlayerPlugin : public Plugin
{
    Q_OBJECT
public:
    MediaPlayerPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~MediaPlayerPlugin() override;

    void load() override;
    void unload() override;

private:
    MediaPlayerActivity *act;
};

}

#endif
