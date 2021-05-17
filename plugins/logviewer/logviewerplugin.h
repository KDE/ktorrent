/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTLOGVIEWERPLUGIN_H
#define KTLOGVIEWERPLUGIN_H

#include <interfaces/activity.h>
#include <interfaces/plugin.h>

class QDockWidget;

namespace kt
{
class LogViewer;
class LogPrefPage;
class LogFlags;

enum LogViewerPosition {
    SEPARATE_ACTIVITY = 0,
    DOCKABLE_WIDGET = 1,
    TORRENT_ACTIVITY = 2,
};

/**
 * @author Joris Guisson
 */
class LogViewerPlugin : public Plugin
{
    Q_OBJECT
public:
    LogViewerPlugin(QObject *parent, const QVariantList &args);
    ~LogViewerPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString &version) const override;
    void guiUpdate() override;

private Q_SLOTS:
    void applySettings();

private:
    void addLogViewerToGUI();
    void removeLogViewerFromGUI();

private:
    LogViewer *lv;
    LogPrefPage *pref;
    LogFlags *flags;
    QDockWidget *dock;
    LogViewerPosition pos;
};

}

#endif
