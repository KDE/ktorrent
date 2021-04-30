/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTINFOWIDGETPLUGIN_H
#define KTINFOWIDGETPLUGIN_H

#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>
#include <interfaces/torrentactivityinterface.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class PeerView;
class TrackerView;
class StatusTab;
class FileView;
class ChunkDownloadView;
class IWPrefPage;
class Monitor;
class WebSeedsTab;

/**
@author Joris Guisson
*/
class InfoWidgetPlugin : public Plugin, public ViewListener
{
    Q_OBJECT
public:
    InfoWidgetPlugin(QObject *parent, const QVariantList &args);
    ~InfoWidgetPlugin() override;

    void load() override;
    void unload() override;
    void guiUpdate() override;
    void currentTorrentChanged(bt::TorrentInterface *tc) override;
    bool versionCheck(const QString &version) const override;

    /// Show PeerView in main window
    void showPeerView(bool show);
    /// Show ChunkDownloadView in main window
    void showChunkView(bool show);
    /// Show TrackerView in main window
    void showTrackerView(bool show);
    /// Show WebSeedsTab in main window
    void showWebSeedsTab(bool show);

private:
    void createMonitor(bt::TorrentInterface *tc);

private Q_SLOTS:
    void applySettings();
    void torrentRemoved(bt::TorrentInterface *tc);

private:
    PeerView *peer_view;
    ChunkDownloadView *cd_view;
    TrackerView *tracker_view;
    FileView *file_view;
    StatusTab *status_tab;
    WebSeedsTab *webseeds_tab;
    Monitor *monitor;

    IWPrefPage *pref;
};

}

#endif
