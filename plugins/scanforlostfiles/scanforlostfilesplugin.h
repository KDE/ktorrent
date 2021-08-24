/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCANFORLOSTFILESPLUGIN_H
#define KTSCANFORLOSTFILESPLUGIN_H

#include <interfaces/plugin.h>

class QString;
class QDockWidget;

namespace kt
{
class ScanForLostFilesPrefPage;
class ScanForLostFilesWidget;

enum SFLFPosition {
    SEPARATE_ACTIVITY = 0,
    DOCKABLE_WIDGET = 1,
    TORRENT_ACTIVITY = 2,
};

/**
 * @author Alexander Trufanov <trufanovan@gmail.com>
 * @brief KTorrent ScanForLostFiles plugin
 * Display files in selected folder that do not belong to any torrent.
 */
class ScanForLostFilesPlugin : public Plugin
{
    Q_OBJECT
public:
    ScanForLostFilesPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ScanForLostFilesPlugin() override;

    void load() override;
    void unload() override;

public Q_SLOTS:
    void updateScanForLostFiles();

private:
    void addToGUI();
    void removeFromGUI();

private:
    ScanForLostFilesWidget *m_view = nullptr;
    QDockWidget *m_dock = nullptr;
    ScanForLostFilesPrefPage *m_pref = nullptr;
    SFLFPosition m_pos = SEPARATE_ACTIVITY;
};

}

#endif
