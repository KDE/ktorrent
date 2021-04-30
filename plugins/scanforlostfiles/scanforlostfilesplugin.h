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
    ScanForLostFilesPlugin(QObject *parent, const QVariantList &args);
    ~ScanForLostFilesPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString &version) const override;

public Q_SLOTS:
    void updateScanForLostFiles();

private:
    void addToGUI();
    void removeFromGUI();

private:
    ScanForLostFilesWidget *m_view;
    QDockWidget *m_dock;
    ScanForLostFilesPrefPage *m_pref;
    SFLFPosition m_pos;
};

}

#endif
