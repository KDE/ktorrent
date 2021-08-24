/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSEARCHPLUGIN_H
#define KTSEARCHPLUGIN_H

#include <QList>

#include "proxy_helper.h"
#include "searchenginelist.h"
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>

namespace kt
{
class SearchPrefPage;
class SearchActivity;

/**
@author Joris Guisson
*/
class SearchPlugin : public Plugin
{
    Q_OBJECT
public:
    SearchPlugin(QObject *parent, const QVariantList &args);
    ~SearchPlugin() override;

    void load() override;
    void unload() override;

    SearchEngineList *getSearchEngineList() const
    {
        return engines;
    }
    SearchActivity *getSearchActivity() const
    {
        return activity;
    }
    ProxyHelper *getProxy() const
    {
        return proxy;
    }

    void search(const QString &text, int engine, bool external);

private Q_SLOTS:
    void preferencesUpdated();

private:
    SearchActivity *activity = nullptr;
    SearchPrefPage *pref = nullptr;
    SearchEngineList *engines = nullptr;
    ProxyHelper *proxy = nullptr;
};

}

#endif
