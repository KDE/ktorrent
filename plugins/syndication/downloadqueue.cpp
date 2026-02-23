/*
    SPDX-FileCopyrightText: 2025 Aditya Tolikar <ulterno@proton.me>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "downloadqueue.h"

#include <QMetaObject>
#include <QTimer>

#include "ktfeed.h"
#include "linkdownloader.h"
#include "syndicationactivity.h"
#include "syndicationplugin.h"
#include <interfaces/coreinterface.h>
#include <magnet/magnetlink.h>
#include <util/log.h>

/**
 * Time to wait after completion of a download, before continuing to the next download
 * @todo replace with functionality that allows user configuration
 */
constexpr std::chrono::seconds wait_before_item_retry_secs(3);

namespace kt
{
DownloadQueue::DownloadQueue(SyndicationPlugin *plugin, QObject *parent)
    : QObject{parent}
    , m_plugin(plugin)
{
}

DownloadQueue::~DownloadQueue()
{
    for (auto &iter : download_queues) {
        for (auto &queued_download : iter) {
            if (queued_download.created) {
                queued_download.downloader->kill();
                delete queued_download.downloader;
            }
        }
    }
}

void DownloadQueue::downloadLink(const QUrl &url, const QString &group, const QString &location, const QString &move_on_completion, bool silently, Feed *sender)
{
    if (!url.isValid()) {
        bt::Out(SYS_SYN | LOG_NOTICE) << "Feed asked to add Invalid Url to download queue: " << sender->feedUrl() << bt::endl;
        sender->itemDownloadResponse(url, Feed::Status::FAILED_TO_DOWNLOAD);
        return;
    }
    enum class AddDlOps {
        None = 0,
        StartDownload
    };
    const std::function<void(AddDlOps)> addNewDl{[&](const AddDlOps ops = AddDlOps::None) -> void {
        QueuedDownload qd{.link_params{
                              .url = url,
                              .verbose = !silently,
                              .group = group,
                              .location = location,
                              .move_on_completion = move_on_completion,
                          },
                          .feed = sender};
        if (ops == AddDlOps::StartDownload) {
            createLinkDownloader(qd);
        }
        download_queues[url.host()].append(std::move(qd));
    }};
    if (url.scheme() == QLatin1StringView("magnet")) {
        MagnetLinkLoadOptions options;
        options.silently = silently;
        options.group = group;
        options.location = location;
        options.move_on_completion = move_on_completion;
        m_plugin->getCore()->load(bt::MagnetLink(url), options);
        return;
    }
    const QString url_host_name = url.host();
    if (!download_queues.contains(url_host_name)) {
        // remote host not occupied -> start download and save entry
        addNewDl(AddDlOps::StartDownload);
        return;
    }
    // remote host occupied
    sender->itemDownloadResponse(url, Feed::Status::AWAITING_RESOURCE);

    // Find queue entry corresponding to url
    auto &host_queue = download_queues[url_host_name];
    auto match = std::find_if(host_queue.begin(), host_queue.end(), [&url](QueuedDownload &dl_entry) -> bool {
        return dl_entry.link_params.url == url;
    });
    // Operate on matching entry
    if (match != host_queue.end()) {
        if ((!match->link_params.verbose)) {
            // If verbose is expected, then set it
            match->link_params.verbose = !silently;
        }
        // If caller has new link_params, update them
        match->link_params.group = group;
        match->link_params.location = location;
        match->link_params.move_on_completion = move_on_completion;
    } else {
        // Make new and save to queue
        addNewDl(AddDlOps::None);
        // Pass to the deciding function
        continueToNextDownload(url_host_name);
    }
}

void DownloadQueue::createLinkDownloader(QueuedDownload &download_entry)
{
    LinkDownloader *dlr = new LinkDownloader(m_plugin->getCore(), download_entry.link_params);
    download_entry.created = true;
    download_entry.downloader = dlr;
    QMetaObject::Connection signal_connection = connect(dlr, &LinkDownloader::finished, this, &DownloadQueue::handleLinkDownloadFinished);
    connect(dlr, &LinkDownloader::entryDownloadFinished, this, [this, signal_connection](LinkDownloader *downloader, bool success) {
        disconnect(signal_connection);
        handleLinkDownloadFinished(downloader, success);
    });
    dlr->start();
}

void DownloadQueue::continueToNextDownload(const QString &hostname)
{
    // Find queue entry corresponding to url
    auto &host_queue = download_queues[hostname];
    if (host_queue.isEmpty()) {
        bt::Out(SYS_SYN | LOG_DEBUG) << "Queue for " << hostname << " empty: " << !download_queues.contains(hostname) << bt::endl;
        return;
    }
    // Run the oldest (FIFO)
    auto run_next = host_queue.begin();
    for (auto iter = host_queue.begin(); iter != host_queue.end(); iter++) {
        // Make sure there is no double running
        if (iter->created) {
            // ToDo: Increment semaphore for current hostname
            bt::Out(SYS_SYN | LOG_DEBUG) << "Attempted new download when host parallel quota is full: " << iter->link_params.url << bt::endl;
            return;
        }
        // Check priority to decide which one to run
        if ((!run_next->link_params.verbose) && (iter->link_params.verbose)) {
            run_next = iter;
        }
    }
    createLinkDownloader(*run_next);
}

void DownloadQueue::handleLinkDownloadFinished(LinkDownloader *downloader, bool success)
{
    const QString hostName = downloader->getUrl().host();
    auto &host_queue = download_queues[hostName];
    auto match = std::find_if(host_queue.begin(), host_queue.end(), [downloader](QueuedDownload &dl_entry) -> bool {
        return dl_entry.downloader == downloader;
    });
    if (match == host_queue.end()) {
        bt::Out(SYS_SYN | LOG_IMPORTANT) << "Unable to find matching entry for finished downloader" << bt::endl;
        return;
    }
    if (success) {
        // Download successful
        if (!match->feed.isNull()) {
            match->feed->itemDownloadResponse(match->link_params.url, Feed::OK);
        }
        host_queue.erase(match);
        if (host_queue.isEmpty()) {
            download_queues.remove(hostName);
        }
    } else {
        // Download Failed
        bool retry = false;
        if (!match->feed.isNull()) {
            retry = match->feed->itemDownloadResponse(match->link_params.url, Feed::FAILED_TO_DOWNLOAD);
        }
        if (retry) {
            // Clear the previous state to let it be created the next try
            match->created = false;
            match->downloader = nullptr;
        } else {
            host_queue.erase(match);
            if (host_queue.isEmpty()) {
                download_queues.remove(hostName);
            }
        }
    }
    const QString host_name = match->link_params.url.host();
    QTimer::singleShot(wait_before_item_retry_secs, this, [this, host_name]() {
        continueToNextDownload(host_name);
    });
}
}
