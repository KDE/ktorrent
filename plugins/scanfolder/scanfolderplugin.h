/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCANFOLDERPLUGIN_H
#define KTSCANFOLDERPLUGIN_H

#include <interfaces/plugin.h>

class QString;

namespace kt
{
class ScanFolderPrefPage;
class TorrentLoadQueue;
class ScanThread;

/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief KTorrent ScanFolder plugin
 * Automatically scans selected folder for torrent files and loads them.
 */
class ScanFolderPlugin : public Plugin
{
    Q_OBJECT
public:
    ScanFolderPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ScanFolderPlugin() override;

    void load() override;
    void unload() override;

public Q_SLOTS:
    void updateScanFolders();

private:
    ScanFolderPrefPage *pref = nullptr;
    TorrentLoadQueue *tlq = nullptr;
    ScanThread *scanner = nullptr;
};

}

#endif
