/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCRIPTINGPLUGIN_H
#define KTSCRIPTINGPLUGIN_H

#include <interfaces/plugin.h>

class KJob;

namespace kt
{
class Script;
class ScriptManager;
class ScriptModel;

/**
    @author
*/
class ScriptingPlugin : public Plugin
{
    Q_OBJECT
public:
    ScriptingPlugin(QObject *parent, const QVariantList &args);
    ~ScriptingPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString &version) const override;

private:
    void scriptDownloadFinished(KJob *job);
    void loadScripts();
    void saveScripts();
    Script *loadScriptDir(const QString &dir);

private Q_SLOTS:
    void addScript();
    void removeScript();

private:
    ScriptManager *sman;
    ScriptModel *model;
};

}

#endif
